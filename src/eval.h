/*
	shared header file
*/
#ifndef EVAL_H
#define EVAL_H

#include <stdbool.h>
#include <stddef.h>

/*
	list.c
*/

typedef struct _ListNode {
   struct _ListNode * next;
   struct _ListNode * previous;
} ListNode;

typedef struct _List {
   ListNode sentinel;
} List;

void                 list_clear (List *);

ListNode            *list_insert (ListNode *, void *);
void *               list_remove (ListNode *);
ListNode            *list_move (ListNode *, void *, void *);

size_t               list_size (List *);

#define              list_begin(list) ((list) -> sentinel.next)
#define              list_end(list) (& (list) -> sentinel)

#define              list_empty(list) (list_begin (list) == list_end (list))

#define              list_next(iterator) ((iterator) -> next)
#define              list_previous(iterator) ((iterator) -> previous)

#define              list_front(list) ((void *) (list) -> sentinel.next)
#define              list_back(list) ((void *) (list) -> sentinel.previous)

/*
	eval.c
*/

char *strndup(const char *s, size_t len);
char *read_file(char *file);

/*
	lex.c
*/

typedef enum {
	TK_IDENTIFIER,
	TK_NUMBER,
	TK_PUNCTUATION,
	TK_EOF
} TokenType;

typedef struct _Token {
	ListNode node;
	TokenType type;
	char *data;
	int length;
} Token;

static Token      *new_token(TokenType type, char *data, int length);
static bool        is_identifier(char bit);
static bool        is_numeric(char bit);
static bool        is_number(char bit);
static bool        is_space(char bit);
static bool        is_punctuation(char bit);
void               tokenize(char *input, List *tokens);

/*
	parse.c
*/

typedef enum {
	ND_ADD,
	ND_SUB,
	ND_MUL,
	ND_DIV,
	ND_NUMBER,
	ND_CALL
} NodeType;

typedef struct _Node {
	NodeType type;
	struct _Node *left;
	struct _Node *right;
	struct _Node *args;
	Token *token;
} Node;

static Node       *new_node(NodeType type, Token *token);
static Node       *new_node_binary(NodeType type, Token *token, Node *left, Node *right);

static Token      *next(Token **token);
static Token      *prev(Token **current);
static bool        consume_string(Token **current, char *data);
static void        expect_string(Token **current, char *data);
static bool        consume_type(Token **current, TokenType type);
static void        expect_type(Token **current, TokenType type);

static Node       *parse_expr(Token **current);
static Node       *parse_add_sub(Token **current);
static Node       *parse_mul_div(Token **current);
static Node       *parse_primary(Token **current);
Node              *parse(List *tokens);

/*
	gen.c
*/

typedef enum {
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_PUSH,
	OP_CALL
} OpType;

typedef struct _Op {
	ListNode node;
	OpType op;
	double left;
	char *left_string;
} Op;

static void       emit_op(List *program, OpType op);
static void       emit_op_left(List *program, OpType op, int left);
static void       emit_op_left_string(List *program, OpType op, char *left_string);

static void       gen_binary(Node *node, List *program);
static void       gen_number(Node *node, List *program);
static void       gen_call(Node *node, List *program);
static void       visitor(Node *node, List *program);
void              gen(Node *node, List *program);

/* 
	run.c
*/

void              run(List *program);

#endif