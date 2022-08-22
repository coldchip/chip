#include <stdio.h>
#include <stdlib.h>
#include "eval.h"

static Op *emit_op(List *program, OpType op) {
	Op *ins = malloc(sizeof(Op));
	ins->op = op;
	list_insert(list_end(program), ins);
	return ins;
}

static Op *emit_op_left(List *program, OpType op, float left) {
	Op *ins = malloc(sizeof(Op));
	ins->op = op;
	ins->left = left;
	list_insert(list_end(program), ins);
	return ins;
}

static Op *emit_op_left_string(List *program, OpType op, char *left_string) {
	Op *ins = malloc(sizeof(Op));
	ins->op = op;
	ins->left_string = left_string;
	list_insert(list_end(program), ins);
	return ins;
}

static int emit_op_get_counter(List *program) {
	return list_size(program) + 1;
}

static void emit_print(List *program) {
	int line = 1;

	printf("LINE\tOP\tVALUE\n--------------------------\n");

	ListNode *node;
	for(node = list_begin(program); node != list_end(program); node = list_next(node)) {
		Op *ins = (Op*)node;
		switch(ins->op) {
			case OP_LOAD: {
				printf("%i\tLOAD\t%s\n", line, ins->left_string);
			}
			break;
			case OP_STORE: {
				printf("%i\tSTORE\t%s\n", line, ins->left_string);
			}
			break;
			case OP_CMPGT: {
				printf("%i\tCMPGT\n", line);
			}
			break;
			case OP_CMPLT: {
				printf("%i\tCMPLT\n", line);
			}
			break;
			case OP_ADD: {
				printf("%i\tADD\n", line);
			}
			break;
			case OP_SUB: {
				printf("%i\tSUB\n", line);
			}
			break;
			case OP_MUL: {
				printf("%i\tMUL\n", line);
			}
			break;
			case OP_DIV: {
				printf("%i\tDIV\n", line);
			}
			break;
			case OP_PUSH: {
				printf("%i\tPUSH\t%f\n", line, ins->left);
			}
			break;
			case OP_CALL: {
				printf("%i\tCALL\t%s\n", line, ins->left_string);
			}
			break;
			case OP_JMPIFT: {
				printf("%i\tJMPIFT\t%i\n", line, (int)ins->left);
			}
			break;
			case OP_JMP: {
				printf("%i\tJMP\t%i\n", line, (int)ins->left);
			}
			break;
		}

		line++;
	}
}

static void gen_program(Node *node, List *program) {
	List *list = &node->bodylist;
	while(!list_empty(list)) {
		Node *entry = (Node*)list_remove(list_begin(list));
		visitor(entry, program);
	}
}

static void gen_method(Node *node, List *program) {
	List *list = &node->bodylist;
	while(!list_empty(list)) {
		Node *entry = (Node*)list_remove(list_begin(list));
		visitor(entry, program);
	}
}

static void gen_if(Node *node, List *program) {
	int start = emit_op_get_counter(program);

	visitor(node->condition, program);
	emit_op_left(program, OP_PUSH, 0);
	Op *jmp = emit_op_left(program, OP_JMPIFT, 0);
	visitor(node->body, program);
	emit_op_left(program, OP_JMP, start);

	jmp->left = emit_op_get_counter(program);
}

static void gen_while(Node *node, List *program) {
	int start = emit_op_get_counter(program);

	visitor(node->condition, program);
	emit_op_left(program, OP_PUSH, 0);
	Op *jmp = emit_op_left(program, OP_JMPIFT, 0);
	visitor(node->body, program);
	emit_op_left(program, OP_JMP, start);

	jmp->left = emit_op_get_counter(program);
}

static void gen_block(Node *node, List *program) {
	List *list = &node->bodylist;
	while(!list_empty(list)) {
		Node *entry = (Node*)list_remove(list_begin(list));
		visitor(entry, program);
	}
}

static void gen_declaration(Node *node, List *program) {
	visitor(node->body, program);
	char *str = strndup(node->token->data, node->token->length);
	emit_op_left_string(program, OP_STORE, str);
}

static void gen_variable(Node *node, List *program) {
	char *str = strndup(node->token->data, node->token->length);
	emit_op_left_string(program, OP_LOAD, str);
}

static void gen_assign(Node *node, List *program) {
	visitor(node->right, program);
	gen_store(node->left, program);
}

static void gen_store(Node *node, List *program) {
	char *str = strndup(node->token->data, node->token->length);
	emit_op_left_string(program, OP_STORE, str);
}

static void gen_binary(Node *node, List *program) {
	if(node->left) {
		visitor(node->left, program);
	}

	if(node->right) {
		visitor(node->right, program);
	}

	switch(node->type) {
		case ND_GT: {
			emit_op(program, OP_CMPGT);
		}
		break;
		case ND_LT: {
			emit_op(program, OP_CMPLT);
		}
		break;
		case ND_ADD: {
			emit_op(program, OP_ADD);
		}
		break;
		case ND_SUB: {
			emit_op(program, OP_SUB);
		}
		break;
		case ND_MUL: {
			emit_op(program, OP_MUL);
		}
		break;
		case ND_DIV: {
			emit_op(program, OP_DIV);
		}
		break;
	}
}

static void gen_number(Node *node, List *program) {
	char *str = strndup(node->token->data, node->token->length);
	emit_op_left(program, OP_PUSH, atof(str));
	free(str);
}

static void gen_call(Node *node, List *program) {
	if(node->args) {
		visitor(node->args, program);
	}
	char *str = strndup(node->token->data, node->token->length);
	emit_op_left_string(program, OP_CALL, str);
}

static void visitor(Node *node, List *program) {
	switch(node->type) {
		case ND_PROGRAM: {
			gen_program(node, program);
		}
		break;
		case ND_METHOD: {
			gen_method(node, program);
		}
		break;
		case ND_IF: {
			gen_if(node, program);
		}
		break;
		case ND_WHILE: {
			gen_while(node, program);
		}
		break;
		case ND_BLOCK: {
			gen_block(node, program);
		}
		break;
		case ND_DECL: {
			gen_declaration(node, program);
		}
		break;
		case ND_VARIABLE: {
			gen_variable(node, program);
		}
		break;
		case ND_ASSIGN: {
			gen_assign(node, program);
		}
		break;
		case ND_GT:
		case ND_LT:
		case ND_ADD:
		case ND_SUB:
		case ND_MUL:
		case ND_DIV: {
			gen_binary(node, program);
		}
		break;
		case ND_NUMBER: {
			gen_number(node, program);
		}
		break;
		case ND_CALL: {
			gen_call(node, program);
		}
		break;
	}
}

void gen(Node *node, List *program) {
	list_clear(program);
	visitor(node, program);
	emit_print(program);

}