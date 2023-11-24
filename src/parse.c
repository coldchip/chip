#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "chip.h"

Node *new_node(NodeType type, Token *token) {
	Node *node  = malloc(sizeof(Node));
	node->type  = type;
	node->token = token;
	node->left  = NULL;
	node->right = NULL;

	list_clear(&node->bodylist);

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
	if(equals_string(current, "class")) {
		return true;
	}
	return false;
}

bool is_method(Token **current) {
	if(equals_string(current, "function")) {
		return true;
	}
	return false;
}

/* 
	a = (expr)
*/

bool is_assign(Token **current) {
	Token *state = *current;

	if(parse_expr(current)) {
		if(consume_string(current, "=")) {
			*current = state;			
			return true;
		}
	}
	*current = state;
	return false;
}

static Node *parse_program(Token **current) {
	Node *node = new_node(ND_PROGRAM, NULL);

	while(is_class(current)) {
		list_insert(list_end(&node->bodylist), parse_class(current));
	}

	return node;
}

static Node *parse_class(Token **current) {
	Node *node = new_node(ND_CLASS, NULL);

	expect_type(current, TK_IDENTIFIER);

	node->token = *current;

	expect_type(current, TK_IDENTIFIER);

	expect_string(current, "{");

	while(is_method(current)) {
		list_insert(list_end(&node->bodylist), parse_method(current));
	}

	expect_string(current, "}");

	return node;
}

static Node *parse_method(Token **current) {
	Node *node = new_node(ND_METHOD, NULL);

	expect_type(current, TK_IDENTIFIER);

	node->token = *current;

	expect_type(current, TK_IDENTIFIER);

	expect_string(current, "(");
	node->args = parse_params(current);
	expect_string(current, ")");

	if(consume_string(current, ":")) {
		expect_string(current, "[");
		if(consume_string(current, "static")) {
			node->modifier = MOD_STATIC;
		} else {
			printf("unknown type modifier\n");
		}
		expect_string(current, "]");
	}

	expect_string(current, "{");

	while(!equals_string(current, "}")) {
		list_insert(list_end(&node->bodylist), parse_stmt(current));
	}

	expect_string(current, "}");

	return node;
}

Node *parse_param(Token **current) {
	Node *node = new_node(ND_VARIABLE, *current);
	expect_type(current, TK_IDENTIFIER);

	return node;
}

Node *parse_params(Token **current) {
	Node *node = new_node(ND_PARAM, NULL);
	if(equals_string(current, ")")) {
		return node;
	}

	node->length = 1;

	list_insert(list_end(&node->bodylist), parse_param(current));

	while(!equals_string(current, ")")) {
		expect_string(current, ",");
		list_insert(list_end(&node->bodylist), parse_param(current));
		node->length++;
	}
	return node;
}

Node *parse_arg(Token **current) {
	Node *node = parse_expr(current);
	return node;
}

Node *parse_args(Token **current) {
	Node *node = new_node(ND_ARG, NULL);
	if(equals_string(current, ")")) {
		return node;
	}

	node->length = 1;

	list_insert(list_end(&node->bodylist), parse_arg(current));

	while(!equals_string(current, ")")) {
		expect_string(current, ",");
		list_insert(list_end(&node->bodylist), parse_arg(current));
		node->length++;
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

		while(!consume_string(current, "}")) {
			list_insert(list_end(&node->bodylist), parse_stmt(current));
		}

		return node;
	} else if(consume_string(current, "return")) {
		Node *node = new_node(ND_RETURN, NULL);
		node->body = parse_expr(current);
		expect_string(current, ";");
		return node;
	} else {
		Node *node = parse_expr_stmt(current);
		expect_string(current, ";");
		return node;
	}
}

static Node *parse_expr_stmt(Token **current) {
	if(is_assign(current)) {
		Node *node = new_node(ND_ASSIGN, NULL);
		node->left = parse_expr(current);
		if(consume_string(current, "=")) {
			node->right = parse_expr(current);
		}

		return node;
	} else {
		Node *node = new_node(ND_EXPR, NULL);
		node->body = parse_expr(current);

		return node;
	}
}

Node *parse(List *tokens) {
	Token *current = (Token*)list_begin(tokens);
	return parse_program(&current);
}