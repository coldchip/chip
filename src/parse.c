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

bool is_type(Token **current) {
	if(equals_string(current, "int")) {
		return true;
	}
	return false;
}

bool is_function(Token **current) {
	if(equals_string(current, "function")) {
		return true;
	}
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

static Node *parse_program(List *varlist, Token **current) {
	Node *node = new_node(ND_PROGRAM, NULL);

	while(is_function(current)) {
		list_insert(list_end(&node->bodylist), parse_function(varlist, current));
	}

	return node;
}

static Node *parse_function(List *varlist, Token **current) {
	Node *node = new_node(ND_FUNCTION, NULL);

	expect_type(current, TK_IDENTIFIER);

	node->token = *current;

	expect_type(current, TK_IDENTIFIER);

	expect_string(current, "(");
	node->args = parse_params(varlist, current);
	expect_string(current, ")");

	expect_string(current, "{");

	while(!equals_string(current, "}")) {
		list_insert(list_end(&node->bodylist), parse_stmt(varlist, current));
	}

	expect_string(current, "}");

	node->size = total_var_size(varlist);

	return node;
}

Node *parse_param(List *varlist, Token **current) {
	VarType type = parse_type(current);

	Node *node = new_node(ND_VARIABLE, *current);
	node->offset = total_var_size(varlist);
	node->size = 8;

	Token *token = *current;

	if(get_var(varlist, strndup(token->data, token->length))) {
		printf("redefinition of variable '%.*s'\n", (*current)->length, (*current)->data);
		exit(1);
	}

	Var *var = malloc(sizeof(Var));
	var->name = strndup(token->data, token->length);
	var->size = 8;
	list_insert(list_end(varlist), var);

	expect_type(current, TK_IDENTIFIER);

	return node;
}

Node *parse_params(List *varlist, Token **current) {
	Node *node = new_node(ND_PARAM, NULL);
	if(equals_string(current, ")")) {
		return node;
	}

	node->length = 1;

	list_insert(list_end(&node->bodylist), parse_param(varlist, current));

	while(!equals_string(current, ")")) {
		expect_string(current, ",");
		list_insert(list_end(&node->bodylist), parse_param(varlist, current));
		node->length++;
	}
	return node;
}

Node *parse_arg(List *varlist, Token **current) {
	Node *node = parse_expr(varlist, current);
	return node;
}

Node *parse_args(List *varlist, Token **current) {
	Node *node = new_node(ND_ARG, NULL);
	if(equals_string(current, ")")) {
		return node;
	}

	node->length = 1;

	list_insert(list_end(&node->bodylist), parse_arg(varlist, current));

	while(!equals_string(current, ")")) {
		expect_string(current, ",");
		list_insert(list_end(&node->bodylist), parse_arg(varlist, current));
		node->length++;
	}
	return node;
}

static Node *parse_stmt(List *varlist, Token **current) {
	if(consume_string(current, "if")) {
		Node *node = new_node(ND_IF, NULL);
		expect_string(current, "(");
		node->condition = parse_expr(varlist, current);
		expect_string(current, ")");
		node->body = parse_stmt(varlist, current);
		if(consume_string(current, "else")) {
			node->alternate = parse_stmt(varlist, current);
		}
		return node;
	} else if(consume_string(current, "while")) {
		Node *node = new_node(ND_WHILE, NULL);

		expect_string(current, "(");
		node->condition = parse_expr(varlist, current);
		expect_string(current, ")");
		node->body = parse_stmt(varlist, current);
		return node;
	} else if(consume_string(current, "{")) {
		Node *node = new_node(ND_BLOCK, NULL);

		while(!consume_string(current, "}")) {
			list_insert(list_end(&node->bodylist), parse_stmt(varlist, current));
		}

		return node;
	} else if(consume_string(current, "return")) {
		Node *node = new_node(ND_RETURN, NULL);
		node->body = parse_expr(varlist, current);
		expect_string(current, ";");
		return node;
	} else if(is_type(current)) {
		Node *node = parse_declaration(varlist, current);
		expect_string(current, ";");
		return node;
	} else {
		Node *node = parse_expr(varlist, current);
		expect_string(current, ";");
		return node;
	}
}

static Node *parse_declaration(List *varlist, Token **current) {
	VarType type = parse_type(current);

	Token *token = *current;
	Node *node = new_node(ND_DECL, token);
	node->offset = total_var_size(varlist);
	node->size = type;

	if(get_var(varlist, strndup(token->data, token->length))) {
		printf("redefinition of variable '%.*s'\n", (*current)->length, (*current)->data);
		exit(1);
	}

	Var *var = malloc(sizeof(Var));
	var->name = strndup(token->data, token->length);
	var->size = type;
	list_insert(list_end(varlist), var);

	expect_type(current, TK_IDENTIFIER);

	if(consume_string(current, "=")) {
		node->body = parse_expr(varlist, current);
	}

	return node;
}

static VarType parse_type(Token **current) {
	if(consume_string(current, "int")) {
		return TY_INT;
	} else {
		fprintf(stderr, "unknown type %s\n", strndup((*current)->data, (*current)->length));
		exit(1);
	}
}

Node *parse(List *tokens) {
	List varlist;
	list_clear(&varlist);

	Token *current = (Token*)list_begin(tokens);
	return parse_program(&varlist, &current);
}