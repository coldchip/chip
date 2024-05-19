#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "chip.h"
#include "tokenize.h"
#include "parse.h"

Node *new_node(NodeType type, Token *token) {
	Node *node  = malloc(sizeof(Node));
	node->type  = type;
	node->token = token;
	node->data_type = NULL;
	node->left  = NULL;
	node->right = NULL;
	node->index = NULL;
	node->body  = NULL;
	node->alternate = NULL;
	node->condition = NULL;
	node->args = NULL;
	node->array_depth = 0;

	node->ty = NULL;

	node->method = NULL;

	node->offset = 0;

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

bool is_import(Token **current) {
	if(equals_string(current, "import")) {
		return true;
	}
	return false;
}

bool is_class(Token **current) {
	if(equals_string(current, "class")) {
		return true;
	}
	return false;
}

bool is_method(Token **current) {
	if(equals_string(current, "method")) {
		return true;
	}
	return false;
}

/*
	<identifier>()
*/

bool is_call(Token **current) {
	Token *state = *current;

	if(consume_type(current, TK_IDENTIFIER)) {
		if(consume_string(current, "(")) {
			*current = state;
			return true;
		}
	}
	*current = state;
	return false;
}

/* 
	<type> a = (expr)
*/

bool is_declaration(Token **current) {
	Token *state = *current;

	if(consume_type(current, TK_IDENTIFIER)) {
		while(consume_string(current, "[")) {
			if(!consume_string(current, "]")) {
				*current = state;
				return false;
			}
		}
		if(consume_type(current, TK_IDENTIFIER)) {
			*current = state;
			return true;
		}
	}
	*current = state;
	return false;
}

/*
	type?[]?...[]
*/

Node *parse_type(Token **current) {
	Node *node = new_node(ND_TYPE, *current);
	expect_type(current, TK_IDENTIFIER);
	while(consume_string(current, "[")) {
		node->array_depth++;
		expect_string(current, "]");
	}

	return node;
}

static Node *parse_program(Token **current) {
	Node *node = new_node(ND_PROGRAM, NULL);

	while((*current)->type != TK_EOF) {
		if(is_import(current)) {
			list_insert(list_end(&node->bodylist), parse_import(current));
		} else if(is_class(current)) {
			list_insert(list_end(&node->bodylist), parse_class(current));
		} else {
			printf("error: import or class expected\n");
			exit(1);
		}
	}

	return node;
}

static Node *parse_import(Token **current) {
	Node *node = new_node(ND_IMPORT, NULL);

	expect_string(current, "import");
	char *module = (*current)->data;
	expect_type(current, TK_IDENTIFIER);
	expect_string(current, ";");

	char filename[1024];
	sprintf(filename, "libchip/%s.chip", module);

	char *input = read_file(filename);
	List tokens;
	list_clear(&tokens);
	tokenize(input, &tokens);
	node->body = parse(&tokens);

	return node;
}

static Node *parse_class(Token **current) {
	Node *node = new_node(ND_CLASS, NULL);

	expect_string(current, "class");

	node->token = *current;

	expect_type(current, TK_IDENTIFIER);

	expect_string(current, "{");

	while(!equals_string(current, "}")) {
		if(is_method(current)) {
			list_insert(list_end(&node->bodylist), parse_method(current));
		} else {
			list_insert(list_end(&node->bodylist), parse_class_declaration(current));
		}
	}

	expect_string(current, "}");

	return node;
}

/*
	method <identifier>(<args>) {
		
	}

	method operator<op>() {
		
	}
*/

static Node *parse_method(Token **current) {
	Node *node = new_node(ND_METHOD, NULL);

	expect_string(current, "method");

	if(consume_string(current, "operator")) {
		node->token = *current;
		expect_type(current, TK_PUNCTUATION);
	} else {
		node->token = *current;
		expect_type(current, TK_IDENTIFIER);
	}

	expect_string(current, "(");
	node->args = parse_params(current);
	expect_string(current, ")");

	expect_string(current, ":");

	node->data_type = parse_type(current);

	expect_string(current, "{");

	while(!equals_string(current, "}")) {
		list_insert(list_end(&node->bodylist), parse_stmt(current));
	}

	expect_string(current, "}");

	return node;
}

Node *parse_class_declaration(Token **current) {
	Node *type = parse_type(current);

	Node *node = new_node(ND_CLASS_DECL, *current);
	node->data_type = type;
	expect_type(current, TK_IDENTIFIER);
	expect_string(current, ";");

	return node;
}

Node *parse_param(Token **current) {
	Node *type = parse_type(current);

	Node *node = new_node(ND_VARIABLE, *current);
	node->data_type = type;
	expect_type(current, TK_IDENTIFIER);

	return node;
}

Node *parse_params(Token **current) {
	Node *node = new_node(ND_PARAM, NULL);
	if(equals_string(current, ")")) {
		return node;
	}

	list_insert(list_end(&node->bodylist), parse_param(current));

	while(!equals_string(current, ")")) {
		expect_string(current, ",");
		list_insert(list_end(&node->bodylist), parse_param(current));
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

	list_insert(list_end(&node->bodylist), parse_arg(current));

	while(!equals_string(current, ")")) {
		expect_string(current, ",");
		list_insert(list_end(&node->bodylist), parse_arg(current));
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

		while(!equals_string(current, "}")) {
			list_insert(list_end(&node->bodylist), parse_stmt(current));
		}

		expect_string(current, "}");

		return node;
	} else if(consume_string(current, "return")) {
		Node *node = new_node(ND_RETURN, NULL);
		if(!equals_string(current, ";")) {
			node->body = parse_expr(current);
		}
		expect_string(current, ";");
		return node;
	} else if(is_declaration(current)) {
		Node *type = parse_type(current);

		Node *node = new_node(ND_DECL, *current);
		node->data_type = type;

		expect_type(current, TK_IDENTIFIER);

		if(equals_string(current, "=")) {
			prev(current);
			node->body = parse_expr(current);
		}

		expect_string(current, ";");

		return node;
	} else {
		Node *node = new_node(ND_EXPR, NULL);
		node->body = parse_expr(current);
		expect_string(current, ";");

		return node;
	}
}

Node *parse(List *tokens) {
	Token *current = (Token*)list_begin(tokens);
	return parse_program(&current);
}