#include <stdio.h>
#include "eval.h"

static void gen_binary(Node *node) {
	if(node->left) {
		visitor(node->left);
	}

	if(node->right) {
		visitor(node->right);
	}

	switch(node->type) {
		case ND_ADD: {
			printf("add\n");
		}
		break;
		case ND_SUB: {
			printf("sub\n");
		}
		break;
		case ND_MUL: {
			printf("mul\n");
		}
		break;
		case ND_DIV: {
			printf("div\n");
		}
		break;
	}
}

static void gen_number(Node *node) {
	printf("push %.*s\n", node->token->length, node->token->data);
}

static void gen_call(Node *node) {
	if(node->args) {
		visitor(node->args);
	}
	printf("call %.*s\n", node->token->length, node->token->data);
}

static void visitor(Node *node) {
	switch(node->type) {
		case ND_ADD:
		case ND_SUB:
		case ND_MUL:
		case ND_DIV: {
			gen_binary(node);
		}
		break;
		case ND_NUMBER: {
			gen_number(node);
		}
		break;
		case ND_CALL: {
			gen_call(node);
		}
		break;
	}
}

void gen(Node *node) {
	visitor(node);
}