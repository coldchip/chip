#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokenize.h"
#include "parse.h"
#include "varscope.h"
#include "type.h"

Node *parse_expr(Token **current) {
	Node *node = parse_assign(current);
	return node;
}

static Node *parse_assign(Token **current) {
	Node *node = parse_or(current);
	if(consume_string(current, "=")) {
		return new_node_binary(ND_ASSIGN, NULL, node, parse_assign(current));
	}
	return node;
}

static Node *parse_or(Token **current) {
	Node *node = parse_and(current);
	for(;;) {
		if(consume_string(current, "||")) {
			node = new_node_binary(ND_OR, NULL, node, parse_and(current));
			continue;
		}
		return node;
	}
}

static Node *parse_and(Token **current) {
	Node *node = parse_equality(current);
	for(;;) {
		if(consume_string(current, "&&")) {
			node = new_node_binary(ND_AND, NULL, node, parse_equality(current));
			continue;
		}
		return node;
	}
}

static Node *parse_equality(Token **current) {
	Node *node = parse_relational(current);
	for(;;) {
		if(consume_string(current, "==")) {
			node = new_node_binary(ND_EQ, NULL, node, parse_relational(current));
			continue;
		}
		return node;
	}
}

static Node *parse_relational(Token **current) {
	Node *node = parse_add_sub(current);
	for(;;) {
		if(consume_string(current, ">")) {
			node = new_node_binary(ND_GT, NULL, node, parse_add_sub(current));
			continue;
		}
		if(consume_string(current, "<")) {
			node = new_node_binary(ND_LT, NULL, node, parse_add_sub(current));
			continue;
		}
		return node;
	}
}

static Node *parse_add_sub(Token **current) {
	Node *node = parse_mul_div(current);
	for(;;) {
		if(consume_string(current, "+")) {
			node = new_node_binary(ND_ADD, NULL, node, parse_mul_div(current));
			continue;
		}
		if(consume_string(current, "-")) {
			node = new_node_binary(ND_SUB, NULL, node, parse_mul_div(current));
			continue;
		}
		return node;
	}
}

static Node *parse_mul_div(Token **current) {
	Node *node = parse_unary(current);
	for(;;) {
		if(consume_string(current, "*")) {
			node = new_node_binary(ND_MUL, NULL, node, parse_unary(current));
			continue;
		}
		if(consume_string(current, "/")) {
			node = new_node_binary(ND_DIV, NULL, node, parse_unary(current));
			continue;
		}
		if(consume_string(current, "%")) {
			node = new_node_binary(ND_MOD, NULL, node, parse_unary(current));
			continue;
		}
		return node;
	}
}

static Node *parse_unary(Token **current) {
	if(consume_string(current, "-")) {
		Node *node = new_node(ND_NEG, NULL);
		node->body = parse_postfix(current);
		return node;
	}

	if(consume_string(current, "!")) {
		Node *node = new_node(ND_NOT, NULL);
		node->body = parse_postfix(current);
		return node;
	}

	return parse_postfix(current);
}

static Node *parse_postfix(Token **current) {
	Node *node = parse_primary(current);
	for(;;) {
		Token *token = *current;

		if(consume_string(current, "[")) {
			Node *left = new_node(ND_ARRAYMEMBER, NULL);
			left->index = parse_expr(current);
			left->body = node;
			node = left;

			expect_string(current, "]");

			continue;
		}

		if(consume_string(current, ".")) {
			if(is_call(current)) {
				Node *left = new_node(ND_CALL, *current);
				expect_type(current, TK_IDENTIFIER);
				expect_string(current, "(");
				left->args = parse_args(current);
				expect_string(current, ")");

				left->body = node;
				node = left;

			} else {
				Node *left = new_node(ND_MEMBER, *current);
				left->body = node;
				node = left;
				expect_type(current, TK_IDENTIFIER);
			}

			continue;
		}

		return node;
	}
}

Node *parse_primary(Token **current) {
	Token *token = *current;
	if(consume_string(current, "new")) {
		Token *name = *current;

		Node *type = parse_type(current);

		if(type->array_depth > 0) {
			Node *node = new_node(ND_NEWARRAY, name);
			node->data_type = type;
			
			expect_string(current, "(");
			node->args = parse_expr(current);
			expect_string(current, ")");

			node->array_depth++;

			return node;
		} else {
			Node *node = new_node(ND_NEW, name);
			node->data_type = type;
			expect_string(current, "(");
			node->args = parse_args(current);
			expect_string(current, ")");
			return node;
		}
	} else if(consume_string(current, "syscall")) {
		Node *node = new_node(ND_SYSCALL, NULL);
		expect_string(current, "(");
		node->args = parse_args(current);
		expect_string(current, ")");
		expect_string(current, ":");
		node->data_type = parse_type(current);
		return node;
	} else if(consume_string(current, "(")) {
		Node *node = parse_expr(current);
		expect_string(current, ")");
		return node;
	} else if(consume_type(current, TK_IDENTIFIER)) {
		return new_node(ND_VARIABLE, token);
	} else if(consume_type(current, TK_NUMBER)) {
		if(strstr(token->data, ".") != NULL) {
			return new_node(ND_FLOAT, token);;
		} else {
			return new_node(ND_NUMBER, token);
		}
	} else if(consume_type(current, TK_CHAR)) {
		return new_node(ND_CHAR, token);
	} else if(consume_type(current, TK_STRING)) {
		return new_node(ND_STRING, token);
	} else {
		printf("error, unexpected token '%s', expecting expression\n", (*current)->data);
		exit(1);
	}
}