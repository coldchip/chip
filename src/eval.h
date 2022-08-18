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

static Token      *new_token(TokenType type, char *data, int length, Token *prev);
static bool        is_identifier(char bit);
static bool        is_numeric(char bit);
static bool        is_number(char bit);
static bool        is_space(char bit);
static bool        is_punctuation(char bit);
Token             *tokenize(char *input);

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
static bool        consume_string(Token **tokens, char *data);
static void        expect_string(Token **tokens, char *data);
static bool        consume_type(Token **tokens, TokenType type);
static void        expect_type(Token **tokens, TokenType type);

static Node       *parse_expr(Token **tokens);
static Node       *parse_add_sub(Token **tokens);
static Node       *parse_mul_div(Token **tokens);
static Node       *parse_primary(Token **tokens);
Node              *parse(Token *tokens);

/*
	gen.c
*/

static void       gen_binary(Node *node);
static void       gen_number(Node *node);
static void       gen_call(Node *node);
static void       visitor(Node *node);
void              gen(Node *node);

#endif