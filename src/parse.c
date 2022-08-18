#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "eval.h"

static Node *new_node(NodeType type, Token *token) {
	Node *node  = malloc(sizeof(Node));
	node->type  = type;
	node->token = token;
	node->left  = NULL;
	node->right = NULL;

	return node;
}

static Node *new_node_binary(NodeType type, Token *token, Node *left, Node *right) {
	Node *node = new_node(type, token);

	if(left) {
		node->left = left;
	}

	if(right) {
		node->right = right;
	}

	return node;
}

static Token *next(Token **current) {
	*current = (Token*)list_next((ListNode*)*current);
	return *current;
}

static Token *prev(Token **current) {
	*current = (Token*)list_previous((ListNode*)*current);
	return *current;
}

static bool consume_string(Token **current, char *data) {
	if((*current)->type != TK_EOF && strncmp((*current)->data, data, (*current)->length) == 0) {
		next(current);
		return true;
	}
	return false;
}

static void expect_string(Token **current, char *data) {
	if(!consume_string(current, data)) {
		printf("expected token '%s'\n", data);
		exit(0);
	}
}

static bool consume_type(Token **current, TokenType type) {
	if((*current)->type == type) {
		next(current);
		return true;
	}
	return false;
}

static void expect_type(Token **current, TokenType type) {
	if(!consume_type(current, type)) {
		printf("expected type '%i'\n", type);
		exit(0);
	}
}

static Node *parse_expr(Token **current) {
	return parse_add_sub(current);
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
	Node *node = parse_primary(current);
	for(;;) {
		Token *token = *current;
		if(consume_string(current, "*")) {
			node = new_node_binary(ND_MUL, token, node, parse_primary(current));
			continue;
		}
		if(consume_string(current, "/")) {
			node = new_node_binary(ND_DIV, token, node, parse_primary(current));
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
	} else if(consume_type(current, TK_IDENTIFIER)) {
		Node *node = new_node(ND_CALL, token);
		expect_string(current, "(");
		node->args = parse_expr(current);
		expect_string(current, ")");
		return node;
	} else if(consume_type(current, TK_NUMBER)) {
		return new_node(ND_NUMBER, token);
	} else {
		printf("error, unexpected token '%.*s'\n", (*current)->length, (*current)->data);
		exit(1);
	}
	
}

Node *parse(List *tokens) {
	Token *current = (Token*)list_begin(tokens);
	return parse_expr(&current);
}