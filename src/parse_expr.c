#include <stdio.h>
#include <stdlib.h>
#include "eval.h"

Node *parse_expr(Token **current) {
	return parse_assign(current);
}

static Node *parse_assign(Token **current) {
	Node *node = parse_relational(current);
	Token *token = *current;
	if(consume_string(current, "=")) {
		return new_node_binary(ND_ASSIGN, token, node, parse_assign(current));
	}
	return node;
}

static Node *parse_relational(Token **current) {
	Node *node = parse_add_sub(current);
	for(;;) {
		Token *token = *current;
		if(consume_string(current, ">")) {
			node = new_node_binary(ND_GT, token, node, parse_add_sub(current));
			continue;
		}
		if(consume_string(current, "<")) {
			node = new_node_binary(ND_LT, token, node, parse_add_sub(current));
			continue;
		}
		return node;
	}
}

static Node *parse_add_sub(Token **current) {
	Node *node = parse_mul_div(current);
	for(;;) {
		Token *token = *current;
		if(consume_string(current, "+")) {
			node = new_node_binary(ND_ADD, token, node, parse_mul_div(current));
			continue;
		}
		if(consume_string(current, "-")) {
			node = new_node_binary(ND_SUB, token, node, parse_mul_div(current));
			continue;
		}
		return node;
	}
}

static Node *parse_mul_div(Token **current) {
	Node *node = parse_postfix(current);
	for(;;) {
		Token *token = *current;
		if(consume_string(current, "*")) {
			node = new_node_binary(ND_MUL, token, node, parse_postfix(current));
			continue;
		}
		if(consume_string(current, "/")) {
			node = new_node_binary(ND_DIV, token, node, parse_postfix(current));
			continue;
		}
		return node;
	}
}

static Node *parse_postfix(Token **current) {
	Node *node = parse_primary(current);
	for(;;) {
		Token *token = *current;
		if(consume_string(current, ".")) {
			Node *left = new_node(ND_MEMBER, *current);

			left->body = node;


			node = left;

			expect_type(current, TK_IDENTIFIER);

			continue;
		}
		return node;
	}
}

static Node *parse_primary(Token **current) {
	Token *token = *current;
	if(consume_string(current, "(")) {
		Node *node = parse_expr(current);
		expect_string(current, ")");
		return node;
	} else if(is_call(current)) {
		Node *node = new_node(ND_CALL, token);
		expect_type(current, TK_IDENTIFIER);
		expect_string(current, "(");
		node->args = parse_expr(current);
		expect_string(current, ")");
		return node;
	} else if(consume_type(current, TK_IDENTIFIER)) {
		return new_node(ND_VARIABLE, token);
	} else if(consume_type(current, TK_NUMBER)) {
		return new_node(ND_NUMBER, token);
	} else if(consume_type(current, TK_STRING)) {
		return new_node(ND_STRING, token);
	} else {
		printf("error, unexpected token '%.*s'\n", (*current)->length, (*current)->data);
		exit(1);
	}
}