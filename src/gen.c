#include <stdio.h>
#include <stdlib.h>
#include "eval.h"

static void emit_op(List *program, OpType op) {
	Op *ins = malloc(sizeof(Op));
	ins->op = op;
	list_insert(list_end(program), ins);
}

static void emit_op_left(List *program, OpType op, int left) {
	Op *ins = malloc(sizeof(Op));
	ins->op = op;
	ins->left = left;
	list_insert(list_end(program), ins);
}

static void emit_op_left_string(List *program, OpType op, char *left_string) {
	Op *ins = malloc(sizeof(Op));
	ins->op = op;
	ins->left_string = left_string;
	list_insert(list_end(program), ins);
}

static void emit_gen(List *program) {
	ListNode *node;
	for(node = list_begin(program); node != list_end(program); node = list_next(node)) {
		Op *ins = (Op*)node;
		switch(ins->op) {
			case OP_ADD:
			case OP_SUB:
			case OP_MUL:
			case OP_DIV: {
				printf("%i\n", ins->op);
			}
			break;
			case OP_PUSH: {
				printf("%i %f\n", ins->op, ins->left);
			}
			break;
			case OP_CALL: {
				printf("%i %s\n", ins->op, ins->left_string);
			}
			break;
		}
	}
}

static void gen_binary(Node *node, List *program) {
	if(node->left) {
		visitor(node->left, program);
	}

	if(node->right) {
		visitor(node->right, program);
	}

	switch(node->type) {
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
	emit_op_left(program, OP_PUSH, atoi(str));
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
	emit_gen(program);
}