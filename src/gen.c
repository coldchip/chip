#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "chip.h"
#include "gen.h"

static LabelEntry labels[8192] = {};
int label_counter = 0;

static Op *codes[32768] = {};
int code_counter = 0;

static List constants;

static int rand_string() {
	static int result = 0;
    return result++;
}

LabelEntry emit_get_label(const char *name) {
	for(int i = 0; i < label_counter; ++i) {
		if(strcmp(labels[i].name, name) == 0) {
			return labels[i];
		}
	}
}

LabelEntry emit_label(const char *name) {
	LabelEntry label = {
		.line = emit_op_get_counter(),
	};
	strcpy(label.name, name);

	labels[label_counter++] = label;

	return label;
}

static Op *emit_op(OpType op) {
	Op *ins = malloc(sizeof(Op));
	ins->op = op;
	ins->left_label = NULL;
	ins->has_left = false;

	codes[code_counter++] = ins;

	return ins;
}

static Op *emit_op_left(OpType op, uint64_t left) {
	Op *ins = malloc(sizeof(Op));
	ins->op = op;
	ins->left = left;
	ins->left_label = NULL;
	ins->has_left = true;

	codes[code_counter++] = ins;

	return ins;
}

static Op *emit_op_left_label(OpType op, const char *left) {
	Op *ins = malloc(sizeof(Op));
	ins->op = op;
	ins->left_label = strdup(left);
	ins->has_left = true;

	codes[code_counter++] = ins;

	return ins;
}

static int emit_constant(List *list, char *data, bool obfuscated) {
	int i = 0;
	for(ListNode *c = list_begin(list); c != list_end(list); c = list_next(c)) {
		Constant *constant = (Constant*)c;
		if(strcmp(data, constant->data) == 0 && constant->obfuscated == obfuscated) {
			return i;
		}
		i++;
	}

	Constant *constant = malloc(sizeof(Constant));
	constant->data = data;
	constant->obfuscated = obfuscated;
	list_insert(list_end(list), constant);
	return list_size(list) - 1;
}

static int emit_op_get_counter() {
	return code_counter + 1;
}

static void emit_file(List *constants) {
	Ty *c = type_get("Main");
	if(!c) {
		printf("entry point class Main not found\n");
		exit(1);
	}

	TyMethod *m = type_get_method(c, "main");
	if(!m) {
		printf("entry point method main not found\n");
		exit(1);
	}

	FILE *prg = fopen("~prg.out", "wb");

	char entry_label[256];
	sprintf(entry_label, "SUB_%p_%s", m, m->name);
	LabelEntry entry = emit_get_label(entry_label);

	fwrite(&entry.line, sizeof(int), 1, prg);

	fwrite(&code_counter, sizeof(int), 1, prg);

	for(int i = 0; i < code_counter; i++) {
		Op *ins = codes[i];

		if(ins->left_label) {
			LabelEntry label = emit_get_label(ins->left_label);
			ins->left = label.line;
		}

		char op = ins->op;

		if(ins->has_left) {
			op = op | (1 << 7);
		}
				
		fwrite(&op, sizeof(char), 1, prg);
		if(ins->has_left) {
			fwrite(&ins->left, sizeof(uint64_t), 1, prg);
		}
	}

	fclose(prg);

	FILE *cst = fopen("~cst.out", "wb");

	int constants_size = list_size(constants);
	fwrite(&constants_size, sizeof(constants_size), 1, cst);

	for(ListNode *c = list_begin(constants); c != list_end(constants); c = list_next(c)) {
		Constant *constant = (Constant*)c;

		if(
			constant->obfuscated && 
			!strcasecmp(constant->data, "this") == 0 && 
			!strcasecmp(constant->data, "count") == 0
		) {

			char obfuscated2[1024] = "";

			int constant_size = strlen(obfuscated2);
			fwrite(&constant_size, sizeof(constant_size), 1, cst);
			fwrite(obfuscated2, sizeof(char), constant_size, cst);
		} else {

			int constant_size = strlen(constant->data);

			fwrite(&constant_size, sizeof(constant_size), 1, cst);
			fwrite(constant->data, sizeof(char), constant_size, cst);
		}
	}

	fclose(cst);

	FILE *fp = fopen("a.out", "wb");

	char magic[8] = "CHIP";
	fwrite(magic, sizeof(char), 8, fp);

	prg = fopen("~prg.out", "rb");
	fseek(prg, 0, SEEK_END);
	int prg_size = ftell(prg);
	fseek(prg, 0, SEEK_SET);

	fwrite(&prg_size, sizeof(prg_size), 1, fp);

	for(int i = 0; i < prg_size; ++i) {
		char ch = fgetc(prg);
		fputc(ch, fp);
	}

	fclose(prg);

	cst = fopen("~cst.out", "rb");
	fseek(cst, 0, SEEK_END);
	int cst_size = ftell(cst);
	fseek(cst, 0, SEEK_SET);

	fwrite(&cst_size, sizeof(cst_size), 1, fp);

	for(int i = 0; i < cst_size; ++i) {
		char ch = fgetc(cst);
		fputc(ch, fp);
	}

	fclose(cst);
	fclose(fp);

	unlink("~prg.out");
	unlink("~cst.out");
}

void emit_asm() {
	for(int pc = 0; pc < code_counter; pc++) {
		Op *ins = codes[pc];

		for(int i = 0; i < label_counter; ++i) {
			if((labels[i].line - 1) == pc) {
				printf("%s:\n", labels[i].name);
				break;
			}
		}

		switch(ins->op) {
			case OP_LOAD: {
				printf("\tload\t%i\n", (ins->left));
			}
			break;
			case OP_STORE: {
				printf("\tstore\t%i\n", (ins->left));
			}
			break;
			case OP_POP: {
				printf("\tpop\t\n");
			}
			break;
			case OP_CMPEQ: {
				printf("\tcmpeq\n");
			}
			break;
			case OP_CMPGT: {
				printf("\tcmpgt\n");
			}
			break;
			case OP_CMPLT: {
				printf("\tcmplt\n");
			}
			break;
			case OP_ADD: {
				printf("\tadd\n");
			}
			break;
			case OP_SUB: {
				printf("\tsub\n");
			}
			break;
			case OP_MUL: {
				printf("\tmul\n");
			}
			break;
			case OP_DIV: {
				printf("\tdiv\n");
			}
			break;
			case OP_MOD: {
				printf("\tmod\n");
			}
			break;
			case OP_OR: {
				printf("\tor\n");
			}
			break;
			case OP_DUP: {
				printf("\tdup\n");
			}
			break;
			case OP_PUSH: {
				printf("\tpush\t%li\n", ins->left);
			}
			break;
			case OP_LOAD_CONST: {
				printf("\tloadconst\t%i\t//%i\n", (int)ins->left, (ins->left));
			}
			break;
			case OP_LOAD_FIELD: {
				printf("\tloadfield\t%i\n", ins->left);
			}
			break;
			case OP_STORE_FIELD: {
				printf("\tstorefield\t%i\n", ins->left);
			}
			break;
			case OP_CALL: {
				printf("\tcall\t%s\n", ins->left_label);
			}
			break;
			case OP_SYSCALL: {
				printf("\tsyscall\n");
			}
			break;
			case OP_NEWO: {
				printf("\tnewo\t%i\n", (ins->left));
			}
			break;
			case OP_NEW_ARRAY: {
				printf("\tnew_array\t%i\n", (ins->left));
			}
			break;
			case OP_LOAD_ARRAY: {
				printf("\tload_array\n");
			}
			break;
			case OP_STORE_ARRAY: {
				printf("\tstore_array\n");
			}
			break;
			case OP_JE: {
				printf("\tje\t%s\n", ins->left_label);
			}
			break;
			case OP_JMP: {
				printf("\tjmp\t%s\n", ins->left_label);
			}
			break;
			case OP_RET: {
				printf("\tret\t\n");
			}
			break;
		}
	}
}

static void gen_program(Node *node) {
	while(!list_empty(&node->bodylist)) {
		Node *entry = (Node*)list_remove(list_begin(&node->bodylist));
		gen_visitor(entry);
	}
}

static void gen_class(Node *node) {
	while(!list_empty(&node->bodylist)) {
		Node *entry = (Node*)list_remove(list_begin(&node->bodylist));
		gen_visitor(entry);
	}
}

static int gen_param(Node *node) {
	int i = 0;
	while(!list_empty(&node->bodylist)) {
		Node *entry = (Node*)list_remove(list_back(&node->bodylist));
		gen_store(entry);
		i++;
	}

	return i;
}

static int gen_arg(Node *node) {
	int i = 0;
	while(!list_empty(&node->bodylist)) {
		Node *entry = (Node*)list_remove(list_back(&node->bodylist));
		gen_visitor(entry);
		i++;
	}

	return i;
}

static void gen_method(Node *node) {
	char label[256];
	sprintf(label, "SUB_%p_%s", node->method, node->method->name);

	emit_label(label);

	gen_visitor(node->args);

	while(!list_empty(&node->bodylist)) {
		Node *entry = (Node*)list_remove(list_begin(&node->bodylist));
		gen_visitor(entry);
	}

	emit_op_left(OP_PUSH, 0);
	emit_op(OP_RET);
}

static void gen_if(Node *node) {
	char condition_label[256];
	sprintf(condition_label, "IB_%i", rand_string());

	char alternate_label[256];
	sprintf(alternate_label, "IA_%i", rand_string());

	char exit_label[256];
	sprintf(exit_label, "IE_%i", rand_string());

	gen_visitor(node->condition);
	emit_op_left(OP_PUSH, 0);
	emit_op_left_label(OP_JE, alternate_label);

	gen_visitor(node->body);

	emit_op_left_label(OP_JMP, exit_label);

	emit_label(alternate_label);

	if(node->alternate) {
		gen_visitor(node->alternate);
	}

	emit_label(exit_label);
}

static void gen_while(Node *node) {
	char body_label[256];
	sprintf(body_label, "WB_%i", rand_string());

	char exit_label[256];
	sprintf(exit_label, "WE_%i", rand_string());

	emit_label(body_label);

	gen_visitor(node->condition);
	emit_op_left(OP_PUSH, 0);
	emit_op_left_label(OP_JE, exit_label);

	gen_visitor(node->body);
	emit_op_left_label(OP_JMP, body_label);

	emit_label(exit_label);
}

static void gen_block(Node *node) {
	while(!list_empty(&node->bodylist)) {
		Node *entry = (Node*)list_remove(list_begin(&node->bodylist));
		gen_visitor(entry);
	}
}

static void gen_variable(Node *node) {
	emit_op_left(OP_LOAD, node->offset);
}

static void gen_member(Node *node) {
	if(node->body) {
		gen_visitor(node->body);
		emit_op_left(OP_LOAD_FIELD, node->offset);
	}
}

static void gen_new(Node *node) {
	emit_op_left(OP_NEWO, node->size);

	if(node->method) {
		emit_op(OP_DUP);
	
		int arg_count = gen_arg(node->args);

		char label[256];
		sprintf(label, "SUB_%p_%s", node->method, node->method->name);

		emit_op_left(OP_PUSH, arg_count);
		emit_op_left_label(OP_CALL, label);

		// discard constructor return
		emit_op(OP_POP);
	}
}

static void gen_new_array(Node *node) {
	gen_visitor(node->args);

	emit_op_left(OP_NEW_ARRAY, emit_constant(&constants, node->token->data, true));
}

static void gen_array_member(Node *node) {
	if(node->body) {
		gen_visitor(node->body);
		gen_visitor(node->index);
		emit_op(OP_LOAD_ARRAY);
	}
}

static void gen_expr(Node *node) {
	gen_visitor(node->body);

	// assignless
	emit_op(OP_POP);
}

static void gen_decl(Node *node) {
	if(node->body) {
		gen_visitor(node->body);
		emit_op_left(OP_STORE, node->offset);
	}
}

static void gen_assign(Node *node) {
	gen_visitor(node->right);
	gen_store(node->left);
}

static void gen_store(Node *node) {
	if(node->body) {
		gen_visitor(node->body);
		if(node->index) {
			/* x[y] = z */
			gen_visitor(node->index);
			emit_op(OP_STORE_ARRAY);
		} else {
			/* x.y = z */
			emit_op_left(OP_STORE_FIELD, node->offset);
		}
	} else {
		/* x = y */
		emit_op_left(OP_STORE, node->offset);
	}

}

static void gen_binary(Node *node) {
	if(node->left) {
		gen_visitor(node->left);
	}

	if(node->right) {
		gen_visitor(node->right);
	}

	switch(node->type) {
		case ND_EQ: {
			emit_op(OP_CMPEQ);
		}
		break;
		case ND_GT: {
			emit_op(OP_CMPGT);
		}
		break;
		case ND_LT: {
			emit_op(OP_CMPLT);
		}
		break;
		case ND_ADD: {
			emit_op(OP_ADD);
		}
		break;
		case ND_SUB: {
			emit_op(OP_SUB);
		}
		break;
		case ND_MUL: {
			emit_op(OP_MUL);
		}
		break;
		case ND_DIV: {
			emit_op(OP_DIV);
		}
		break;
		case ND_MOD: {
			emit_op(OP_MOD);
		}
		break;
		case ND_OR: {
			emit_op(OP_OR);
		}
		break;
	}
}

static void gen_char(Node *node) {
	int t = (int)node->token->data[0];

	emit_op_left(OP_PUSH, (float)node->token->data[0]);
}

static void gen_number(Node *node) {
	emit_op_left(OP_PUSH, atof(node->token->data));
}

static void gen_float(Node *node) {
	emit_op_left(OP_PUSH, atof(node->token->data));
}

static void gen_string(Node *node) {
	emit_op_left(OP_LOAD_CONST, emit_constant(&constants, node->token->data, false));
}

static void gen_return(Node *node) {
	gen_visitor(node->body);
	emit_op(OP_RET);
}

static void gen_call(Node *node) {
	gen_visitor(node->body);

	int arg_count = gen_arg(node->args);

	char label[256];
	sprintf(label, "SUB_%p_%s", node->method, node->method->name);

	emit_op_left(OP_PUSH, arg_count);
	emit_op_left_label(OP_CALL, label);
}

static void gen_syscall(Node *node) {
	gen_visitor(node->args);
	emit_op_left(OP_SYSCALL, list_size(&node->args->bodylist));
}

static void gen_visitor(Node *node) {
	switch(node->type) {
		case ND_PROGRAM: {
			gen_program(node);
		}
		break;
		case ND_CLASS: {
			gen_class(node);
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
		case ND_METHOD: {
			gen_method(node);
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
		case ND_VARIABLE: {
			gen_variable(node);
		}
		break;
		case ND_MEMBER: {
			gen_member(node);
		}
		break;
		case ND_NEW: {
			gen_new(node);
		}
		break;
		case ND_NEWARRAY: {
			gen_new_array(node);
		}
		break;
		case ND_ARRAYMEMBER: {
			gen_array_member(node);
		}
		break;
		case ND_EXPR: {
			gen_expr(node);
		}
		break;
		case ND_DECL: {
			gen_decl(node);
		}
		break;
		case ND_ASSIGN: {
			gen_assign(node);
		}
		break;
		case ND_EQ:
		case ND_GT:
		case ND_LT:
		case ND_ADD:
		case ND_SUB:
		case ND_MUL:
		case ND_DIV:
		case ND_MOD:
		case ND_OR: {
			gen_binary(node);
		}
		break;
		case ND_CHAR: {
			gen_char(node);
		}
		break;
		case ND_NUMBER: {
			gen_number(node);
		}
		break;
		case ND_FLOAT: {
			gen_float(node);
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
		case ND_SYSCALL: {
			gen_syscall(node);
		}
		break;
	}
}

void gen(Node *node) {
	list_clear(&constants);
	gen_visitor(node);

	emit_asm();

	emit_file(&constants);
}