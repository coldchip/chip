#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "eval.h"

Node *new_node(NodeType type, Token *token) {
	Node *node  = malloc(sizeof(Node));
	node->type  = type;
	node->token = token;
	node->left  = NULL;
	node->right = NULL;

	return node;
}

Node *new_node_binary(NodeType type, Token *token, Node *left, Node *right) {
	Node *node = new_node(type, token);

	if(left) {
		node->left = left;
	}

	if(right) {
		node->right = right;
	}

	return node;
}

bool is_class(Token **current) {
	if(consume_string(current, "class")) {
		prev(current);
		return true;
	}
	return false;
}

bool is_method(Token **current) {
	return false;
}

static Node *parse_class(Token **current) {
	Node *node = new_node(ND_CLASS, NULL);

	expect_string(current, "class");
	expect_type(current, TK_IDENTIFIER);
	expect_string(current, "{");

	while(is_method(current)) {
		parse_method(current);
	}

	expect_string(current, "}");

	return node;
}

static Node *parse_method(Token **current) {

}

static Node *parse_program(Token **current) {
	while(is_class(current)) {
		parse_class(current);
	}
}

Node *parse(List *tokens) {
	Token *current = (Token*)list_begin(tokens);
	return parse_program(&current);
}