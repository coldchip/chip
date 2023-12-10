#ifndef PARSE_H
#define PARSE_H

#include <stdbool.h>
#include "tokenize.h"
#include "type.h"

typedef enum {
	MOD_STATIC = (1 << 0),
	MOD_PROTECTED = (1 << 1)
} Modifier;

typedef enum {
	ND_PROGRAM,
	ND_CLASS,
	ND_PARAM,
	ND_ARG,
	ND_METHOD,
	ND_MEMBER,
	ND_NEWARRAY,
	ND_ARRAYMEMBER,
	ND_NEW,
	ND_BLOCK,
	ND_IF,
	ND_WHILE,
	ND_VARIABLE,
	ND_EXPR,
	ND_DECLARATION,
	ND_ASSIGN,
	ND_EQ,
	ND_GT,
	ND_LT,
	ND_ADD,
	ND_SUB,
	ND_MUL,
	ND_DIV,
	ND_MOD,
	ND_OR,
	ND_NUMBER,
	ND_FLOAT,
	ND_CHAR,
	ND_STRING,
	ND_RETURN,
	ND_CALL,
	ND_SYSCALL,
} NodeType;

typedef struct _Node {
	ListNode node;

	NodeType type;
	Ty *data_type;
	TyMethod *method;
	struct _Node *left;
	struct _Node *right;
	struct _Node *args;
	struct _Node *condition;
	struct _Node *index;
	struct _Node *body;
	struct _Node *alternate;

	int length;

	List bodylist;

	Modifier modifier;

	Token *token;
} Node;

Node              *new_node(NodeType type, Token *token);
Node              *new_node_binary(NodeType type, Token *token, Node *left, Node *right);

bool               is_class(Token **current);
bool               is_method(Token **current);
bool               is_call(Token **current);
bool               is_declaration(Token **current);
bool               is_assign(Token **current);

static Node       *parse_program(Token **current);
Ty                *parse_type(Token **current);
static Node       *parse_class(Token **current);
static Node       *parse_method(Token **current);
Node              *parse_class_declaration(Token **current);
Node              *parse_param(Token **current);
Node              *parse_params(Token **current);
Node              *parse_arg(Token **current);
Node              *parse_args(Token **current);
static Node       *parse_stmt(Token **current);
static Node       *parse_expr_stmt(Token **current);

Node              *parse(List *tokens);

Ty                *unfold_member_type(Node *node);
void               normalize_type(Node *node);

/*
	parse_expr.c
*/

Node              *parse_expr(Token **current);
static Node       *parse_or(Token **current);
static Node       *parse_equality(Token **current);
static Node       *parse_relational(Token **current);
static Node       *parse_add_sub(Token **current);
static Node       *parse_mul_div(Token **current);
static Node       *parse_postfix(Token **current);
Node              *parse_primary(Token **current);

#endif