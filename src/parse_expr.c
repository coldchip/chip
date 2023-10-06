#include <stdio.h>
#include <stdlib.h>
#include "chip.h"

Node *parse_expr(List *varlist, Token **current) {
	return parse_assign(varlist, current);
}

static Node *parse_assign(List *varlist, Token **current) {
	Node *node = parse_relational(varlist, current);
	Token *token = *current;
	if(consume_string(current, "=")) {
		return new_node_binary(ND_ASSIGN, token, node, parse_assign(varlist, current));
	}
	return node;
}

static Node *parse_relational(List *varlist, Token **current) {
	Node *node = parse_add_sub(varlist, current);
	for(;;) {
		Token *token = *current;
		if(consume_string(current, ">")) {
			node = new_node_binary(ND_GT, token, node, parse_add_sub(varlist, current));
			continue;
		}
		if(consume_string(current, "<")) {
			node = new_node_binary(ND_LT, token, node, parse_add_sub(varlist, current));
			continue;
		}
		return node;
	}
}

static Node *parse_add_sub(List *varlist, Token **current) {
	Node *node = parse_mul_div(varlist, current);
	for(;;) {
		Token *token = *current;
		if(consume_string(current, "+")) {
			node = new_node_binary(ND_ADD, token, node, parse_mul_div(varlist, current));
			continue;
		}
		if(consume_string(current, "-")) {
			node = new_node_binary(ND_SUB, token, node, parse_mul_div(varlist, current));
			continue;
		}
		return node;
	}
}

static Node *parse_mul_div(List *varlist, Token **current) {
	Node *node = parse_unary(varlist, current);
	for(;;) {
		Token *token = *current;
		if(consume_string(current, "*")) {
			node = new_node_binary(ND_MUL, token, node, parse_unary(varlist, current));
			continue;
		}
		if(consume_string(current, "/")) {
			node = new_node_binary(ND_DIV, token, node, parse_unary(varlist, current));
			continue;
		}
		if(consume_string(current, "%")) {
			node = new_node_binary(ND_MOD, token, node, parse_unary(varlist, current));
			continue;
		}
		return node;
	}
}

static Node *parse_unary(List *varlist, Token **current) {
	if(consume_string(current, "&")) {
		
	}
	return parse_postfix(varlist, current);
}

static Node *parse_postfix(List *varlist, Token **current) {
	Node *node = parse_primary(varlist, current);
	for(;;) {
		Token *token = *current;
		if(consume_string(current, "(")) {

			Node *left = new_node(ND_CALL, token);
			left->args = parse_args(varlist, current);

			left->body = node;
			node = left;

			expect_string(current, ")");


			continue;
		}
		return node;
	}
}

static Node *parse_primary(List *varlist, Token **current) {
	Token *token = *current;
	if(consume_string(current, "(")) {
		Node *node = parse_expr(varlist, current);
		expect_string(current, ")");
		return node;
	} else if(is_call(current)) {
		if(consume_string(current, "__asm__")) {
			expect_string(current, "(");

			Node *node = new_node(ND_ASM, *current);

			expect_type(current, TK_STRING);
			expect_string(current, ")");

			return node;
		} else if(consume_string(current, "__asm__var_addr")) {
			expect_string(current, "(");

			Var *var = get_var(varlist, strndup((*current)->data, (*current)->length));
			if(!var) {
				printf("variable '%s' not defined\n", strndup((*current)->data, (*current)->length));
				exit(1);
			}

			Node *node = new_node(ND_ASM_VAR_ADDR, (*current));
			node->offset = var_pos(varlist, strndup((*current)->data, (*current)->length));
			node->size = var->size;

			expect_type(current, TK_IDENTIFIER);
			expect_string(current, ")");

			return node;
		} else {
			Node *node = new_node(ND_CALL, token);
			node->token = *current;

			expect_type(current, TK_IDENTIFIER);
			expect_string(current, "(");
			node->args = parse_args(varlist, current);
			expect_string(current, ")");
			return node;
		}
	} else if(consume_type(current, TK_IDENTIFIER)) {
		Var *var = get_var(varlist, strndup(token->data, token->length));
		if(!var) {
			printf("variable '%s' not defined\n", strndup(token->data, token->length));
			exit(1);
		}

		Node *node = new_node(ND_VARIABLE, token);

		int displacement = 0;
		if(consume_string(current, "[")) {
			node->index = parse_expr(varlist, current);
			expect_string(current, "]");
		}

		node->offset = var_pos(varlist, strndup(token->data, token->length));
		node->size = var->size;
		return node;
	} else if(consume_type(current, TK_NUMBER)) {
		return new_node(ND_NUMBER, token);
	} else if(consume_type(current, TK_STRING)) {
		return new_node(ND_STRING, token);
	} else {
		fprintf(stderr, "error, unexpected token '%.*s'\n", (*current)->length, (*current)->data);
		exit(1);
	}
}