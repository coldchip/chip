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

static Token *next(Token **tokens) {
	*tokens = (*tokens)->next;
	return *tokens;
}

static bool consume_string(Token **tokens, char *data) {
	if((*tokens)->type != TK_EOF && strncmp((*tokens)->data, data, (*tokens)->length) == 0) {
		next(tokens);
		return true;
	}
	return false;
}

static void expect_string(Token **tokens, char *data) {
	if(!consume_string(tokens, data)) {
		printf("expected token '%s'\n", data);
		exit(0);
	}
}

static bool consume_type(Token **tokens, TokenType type) {
	if((*tokens)->type == type) {
		next(tokens);
		return true;
	}
	return false;
}

static void expect_type(Token **tokens, TokenType type) {
	if(!consume_type(tokens, type)) {
		printf("expected type '%i'\n", type);
		exit(0);
	}
}

static Node *parse_expr(Token **tokens) {
	return parse_add_sub(tokens);
}

static Node *parse_add_sub(Token **tokens) {
	Node *node = parse_mul_div(tokens);
	for(;;) {
		Token *token = *tokens;
		if(consume_string(tokens, "+")) {
			node = new_node_binary(ND_ADD, token, node, parse_mul_div(tokens));
			continue;
		}
		if(consume_string(tokens, "-")) {
			node = new_node_binary(ND_SUB, token, node, parse_mul_div(tokens));
			continue;
		}
		return node;
	}
}

static Node *parse_mul_div(Token **tokens) {
	Node *node = parse_primary(tokens);
	for(;;) {
		Token *token = *tokens;
		if(consume_string(tokens, "*")) {
			node = new_node_binary(ND_MUL, token, node, parse_primary(tokens));
			continue;
		}
		if(consume_string(tokens, "/")) {
			node = new_node_binary(ND_DIV, token, node, parse_primary(tokens));
			continue;
		}
		return node;
	}
}

static Node *parse_primary(Token **tokens) {
	Token *token = *tokens;
	if(consume_string(tokens, "(")) {
		Node *node = parse_expr(tokens);
		expect_string(tokens, ")");
		return node;
	} else if(consume_type(tokens, TK_IDENTIFIER)) {
		Node *node = new_node(ND_CALL, token);
		expect_string(tokens, "(");
		node->args = parse_expr(tokens);
		expect_string(tokens, ")");
		return node;
	} else if(consume_type(tokens, TK_NUMBER)) {
		return new_node(ND_NUMBER, token);
	} else {
		printf("error, unexpected token '%.*s'\n", (*tokens)->length, (*tokens)->data);
		exit(1);
	}
	
}

Node *parse(Token *tokens) {
	return parse_add_sub(&tokens);
}