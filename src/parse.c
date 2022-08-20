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

bool is_typename(Token **current) {
	if(equals_string(current, "int")) {
		return true;
	}
	return false;
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

bool is_call(Token **current) {
	if(consume_type(current, TK_IDENTIFIER)) {
		if(consume_string(current, "(")) {
			prev(current); prev(current);
			return true;
		}
		prev(current);
	}
	return false;
}

static Node *parse_program(Token **current) {
	Node *node = new_node(ND_PROGRAM, NULL);
	list_clear(&node->bodylist);

	while(is_class(current)) {
		list_insert(list_end(&node->bodylist), parse_class(current));
	}

	return node;
}

static Node *parse_class(Token **current) {
	Node *node = new_node(ND_CLASS, NULL);
	list_clear(&node->bodylist);

	expect_string(current, "class");
	expect_type(current, TK_IDENTIFIER);
	expect_string(current, "{");

	while(!consume_string(current, "}")) {
		list_insert(list_end(&node->bodylist), parse_method(current));
	}

	return node;
}

static Node *parse_method(Token **current) {
	Node *node = new_node(ND_METHOD, NULL);
	list_clear(&node->bodylist);

	expect_type(current, TK_IDENTIFIER);

	Token *token = *current;
	expect_type(current, TK_IDENTIFIER);

	expect_string(current, "(");
	expect_string(current, ")");

	expect_string(current, "{");

	while(!consume_string(current, "}")) {
		list_insert(list_end(&node->bodylist), parse_stmt(current));
	}

	return node;
}

static Node *parse_stmt(Token **current) {
	if(consume_string(current, "if")) {
		Node *node = new_node(ND_IF, NULL);

		expect_string(current, "(");
		node->condition = parse_expr(current);
		expect_string(current, ")");
		node->body = parse_stmt(current);
		if(consume_string(current, "else")) {
			node->alternate = parse_stmt(current);
		}
		return node;
	} else if(consume_string(current, "while")) {
		Node *node = new_node(ND_WHILE, NULL);

		expect_string(current, "(");
		node->condition = parse_expr(current);
		expect_string(current, ")");
		node->body = parse_stmt(current);
		return node;
	} else if(consume_string(current, "{")) {
		Node *node = new_node(ND_BLOCK, NULL);

		list_clear(&node->bodylist);

		while(!consume_string(current, "}")) {
			list_insert(list_end(&node->bodylist), parse_stmt(current));
		}

		return node;
	} else if(is_typename(current)) {
		Node *node = parse_declaration(current);
		expect_string(current, ";");
		return node;
	} else {
		Node *node = parse_expr(current);
		expect_string(current, ";");
		return node;
	}
}

static Node *parse_declaration(Token **current) {
	expect_type(current, TK_IDENTIFIER);

	Token *token = *current;
	Node *node = new_node(ND_DECL, token);

	expect_type(current, TK_IDENTIFIER);

	if(consume_string(current, "=")) {
		node->body = parse_expr(current);
	}

	return node;
}

Node *parse(List *tokens) {
	Token *current = (Token*)list_begin(tokens);
	return parse_program(&current);
}