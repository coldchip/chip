#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "chip.h"

FILE *fp = NULL;

static int current_index() {
	static int i = 0;
	return i++;
}

static void gen_program(Node *node) {
	List *list = &node->bodylist;
	while(!list_empty(list)) {
		Node *entry = (Node*)list_remove(list_begin(list));
		visitor(entry);
	}
}

static void gen_param(Node *node) {
	List *list = &node->bodylist;
	while(!list_empty(list)) {
		Node *entry = (Node*)list_remove(list_back(list));
		gen_store(entry);
	}
}

static void gen_arg(Node *node) {
	List *list = &node->bodylist;
	while(!list_empty(list)) {
		Node *entry = (Node*)list_remove(list_back(list));
		visitor(entry);
		fprintf(fp, "\tpush\trax\n");
	}
}

static void gen_function(Node *node) {
	char *name = strndup(node->token->data, node->token->length);
	fprintf(fp, "chip_func_%s:\n", name);
	free(name);

	fprintf(fp, "\tpush\trbp\n");
	fprintf(fp, "\tmov\trbp, rsp\n");

	fprintf(fp, "\tsub\trsp, %i\n", node->size);

	visitor(node->args);

	List *list = &node->bodylist;
	while(!list_empty(list)) {
		Node *entry = (Node*)list_remove(list_begin(list));
		visitor(entry);
	}

	fprintf(fp, "\tmov\trsp, rbp\n");
	fprintf(fp, "\tpop\trbp\n");
	fprintf(fp, "\tret\n\n\n");
}

static void gen_if(Node *node) {
	int label = current_index();

	visitor(node->condition);

	fprintf(fp, "\tmov\trbx, 1\n");
	fprintf(fp, "\tcmp\trbx, rax\n");
	fprintf(fp, "\tjg\tL%i\n", label);

	visitor(node->body);

	fprintf(fp, "L%i:\n", label);
}

static void gen_while(Node *node) {
	int label = current_index();
	int e = current_index();

	fprintf(fp, "L%i:\n", label);

	visitor(node->condition);

	fprintf(fp, "\tmov\trbx, 1\n");
	fprintf(fp, "\tcmp\trbx, rax\n");
	fprintf(fp, "\tjg\tL%i\n", e);

	visitor(node->body);

	fprintf(fp, "\tjmp\tL%i\n", label);
	fprintf(fp, "L%i:\n", e);
}

static void gen_block(Node *node) {
	List *list = &node->bodylist;
	while(!list_empty(list)) {
		Node *entry = (Node*)list_remove(list_begin(list));
		visitor(entry);
	}
}

static void gen_declaration(Node *node) {
	gen_store(node);
}

static void gen_variable(Node *node) {
	if(node->index) {
		fprintf(fp, "\txor rdi, rdi\n");
		visitor(node->index);
		fprintf(fp, "\tmov rdi, rax\n");
		fprintf(fp, "\tmov rax, [rbp-%i+(rdi*8)]\n", node->offset + node->size);
	} else {
		fprintf(fp, "\tmov rax, [rbp-%i]\n", node->offset + node->size);
	}
}

static void gen_assign(Node *node) {
	visitor(node->right);
	gen_store(node->left);
}

static void gen_store(Node *node) {
	if(node->body) {
		visitor(node->body);
	}


	if(node->index) {
		fprintf(fp, "\tpush rax\n");
		fprintf(fp, "\txor rdi, rdi\n");
		visitor(node->index);
		fprintf(fp, "\tmov rdi, rax\n");
		fprintf(fp, "\tpop rax\n");
		fprintf(fp, "\tmov [rbp-%i+(rdi*8)], rax\n", node->offset + node->size);

	} else {
		fprintf(fp, "\tmov [rbp-%i], rax\n", node->offset + node->size);
	}

}

static void gen_binary(Node *node) {
	if(node->left) {
		visitor(node->left);
		fprintf(fp, "\tpush rax\n");
	}

	if(node->right) {
		visitor(node->right);
		fprintf(fp, "\tpush rax\n");
	}

	fprintf(fp, "\tpop rbx\n");
	fprintf(fp, "\tpop rax\n");

	switch(node->type) {
		case ND_GT: {
			fprintf(fp, "\tcmp rax, rbx\n");
			fprintf(fp, "\tsetg al\n");
			fprintf(fp, "\tmovzx rax, al\n");
		}
		break;
		case ND_LT: {
			fprintf(fp, "\tcmp rax, rbx\n");
			fprintf(fp, "\tsetl al\n");
			fprintf(fp, "\tmovzx rax, al\n");
		}
		break;
		case ND_ADD: {
			fprintf(fp, "\tadd rax, rbx\n");
		}
		break;
		case ND_SUB: {
			fprintf(fp, "\tsub rax, rbx\n");
		}
		break;
		case ND_MUL: {
			fprintf(fp, "\timul rax, rbx\n");
		}
		break;
		case ND_DIV: {
			fprintf(fp, "\tcdq\n");
			fprintf(fp, "\tidiv rbx\n");
		}
		break;
		case ND_MOD: {
			fprintf(fp, "\tcdq\n");
			fprintf(fp, "\tidiv rbx\n");
			fprintf(fp, "\tmov rax, rdx\n");
		}
		break;
	}
}

static void gen_number(Node *node) {
	char *str = strndup(node->token->data, node->token->length);

	fprintf(fp, "\tmov rax, %s\n", str);

	free(str);
}

static void gen_string(Node *node) {
	char *str = strndup(node->token->data, node->token->length);
}

static void gen_return(Node *node) {
	visitor(node->body);
}

static void gen_call(Node *node) {
	Node *args = node->args;
	visitor(args);

	char *name = strndup(node->token->data, node->token->length);
	fprintf(fp, "\tcall\tchip_func_%s\n", name);
	free(name);

	fprintf(fp, "\tadd rsp, %i\n", args->length * 8);
}

static void gen_asm(Node *node) {
	char *data = strndup(node->token->data, node->token->length);
	fprintf(fp, "%s", data);
}

static void gen_asm_var_addr(Node *node) {
	fprintf(fp, "\tlea rax, [rbp-%i]\n", node->offset + 8);
}

static void gen_syscall(Node *node) {
	visitor(node->args);
	char *str = strndup(node->token->data, node->token->length);
}

static void visitor(Node *node) {
	switch(node->type) {
		case ND_PROGRAM: {
			gen_program(node);
		}
		break;
		case ND_PARAM: {
			gen_param(node);
		}
		break;
		case ND_ARG: {
			gen_arg(node);
		}
		break;
		case ND_FUNCTION: {
			gen_function(node);
		}
		break;
		case ND_IF: {
			gen_if(node);
		}
		break;
		case ND_WHILE: {
			gen_while(node);
		}
		break;
		case ND_BLOCK: {
			gen_block(node);
		}
		break;
		case ND_DECL: {
			gen_declaration(node);
		}
		break;
		case ND_VARIABLE: {
			gen_variable(node);
		}
		break;
		case ND_ASSIGN: {
			gen_assign(node);
		}
		break;
		case ND_GT:
		case ND_LT:
		case ND_ADD:
		case ND_SUB:
		case ND_MUL:
		case ND_DIV:
		case ND_MOD: {
			gen_binary(node);
		}
		break;
		case ND_NUMBER: {
			gen_number(node);
		}
		break;
		case ND_STRING: {
			gen_string(node);
		}
		break;
		case ND_RETURN: {
			gen_return(node);
		}
		break;
		case ND_CALL: {
			gen_call(node);
		}
		break;
		case ND_ASM: {
			gen_asm(node);
		}
		break;
		case ND_ASM_VAR_ADDR: {
			gen_asm_var_addr(node);
		}
		break;
		case ND_SYSCALL: {
			gen_syscall(node);
		}
		break;
	}
}

void gen(Node *node, List *p) {
	fp = fopen("out.S", "wb+");
	if(!fp) {
		fprintf(stdout, "unable to open output file");
		exit(1);
	}

	fprintf(fp, "global\t_start\n\n");
	fprintf(fp, "_start:\n");
	fprintf(fp, "\tcall\tchip_func_main\n");

	fprintf(fp, "\tmov\trax, 60\n");
	fprintf(fp, "\txor\trdi, rdi\n");
	fprintf(fp, "\tsyscall\n\n\n");

	visitor(node);
	fclose(fp);
}