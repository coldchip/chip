/*
	shared header file
*/
#ifndef CHIP_H
#define CHIP_H

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
	varlist.c
*/

typedef struct _Var {
	ListNode node;
	int size;
	char *name;
	bool is_array;
} Var;

Var                 *get_var(List* varlist, char *name);
int                  total_var_size(List* varlist);
int                  var_pos(List* varlist, char *name);

/*
	eval.c
*/

char                *strndup(const char *s, size_t len);
char                *strdup(const char *s);
char                *read_file(char *file);

/*
	lex.c
*/

typedef enum {
	TK_IDENTIFIER,
	TK_NUMBER,
	TK_STRING,
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

Token             *next(Token **token);
Token             *prev(Token **current);
bool               equals_string(Token **current, char *data);
bool               equals_type(Token **current, TokenType type);
bool               consume_string(Token **current, char *data);
bool               consume_type(Token **current, TokenType type);
void               expect_string(Token **current, char *data);
void               expect_type(Token **current, TokenType type);

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
	TY_INT = 8
} VarType;

typedef enum {
	ND_PROGRAM,
	ND_PARAM,
	ND_ARG,
	ND_FUNCTION,
	ND_BLOCK,
	ND_IF,
	ND_WHILE,
	ND_DECL,
	ND_VARIABLE,
	ND_ASSIGN,
	ND_GT,
	ND_LT,
	ND_ADD,
	ND_SUB,
	ND_MUL,
	ND_DIV,
	ND_MOD,
	ND_NUMBER,
	ND_STRING,
	ND_RETURN,
	ND_CALL,
	ND_ASM,
	ND_ASM_VAR_ADDR,
	ND_SYSCALL,
} NodeType;

typedef struct _Node {
	ListNode node;

	NodeType type;
	struct _Node *left;
	struct _Node *right;
	struct _Node *args;
	struct _Node *condition;
	struct _Node *body;
	struct _Node *alternate;

	struct _Node *index;
	int offset;
	int size;
	int length;

	List bodylist;

	Token *token;
} Node;

Node              *new_node(NodeType type, Token *token);
Node              *new_node_binary(NodeType type, Token *token, Node *left, Node *right);

bool 					 is_type(Token **current);
bool               is_class(Token **current);
bool               is_function(Token **current);
bool               is_call(Token **current);

static Node       *parse_program(List *varlist, Token **current);
static Node       *parse_function(List *varlist, Token **current);
Node              *parse_param(List *varlist, Token **current);
Node              *parse_params(List *varlist, Token **current);
Node              *parse_arg(List *varlist, Token **current);
Node              *parse_args(List *varlist, Token **current);
static Node       *parse_stmt(List *varlist, Token **current);
static Node       *parse_declaration(List *varlist, Token **current);
static VarType     parse_type(Token **current);

Node              *parse(List *tokens);

/*
	parse_expr.c
*/

Node              *parse_expr(List *varlist, Token **current);
static Node       *parse_assign(List *varlist, Token **current);
static Node       *parse_relational(List *varlist, Token **current);
static Node       *parse_add_sub(List *varlist, Token **current);
static Node       *parse_mul_div(List *varlist, Token **current);
static Node       *parse_unary(List *varlist, Token **current);
static Node       *parse_postfix(List *varlist, Token **current);
static Node       *parse_primary(List *varlist, Token **current);

/*
	gen.c
*/

static int        current_index();
static void       gen_program(Node *node);
static void       gen_class(Node *node);
static void       gen_param(Node *node);
static void       gen_arg(Node *node);
static void       gen_function(Node *node);
static void       gen_if(Node *node);
static void       gen_while(Node *node);
static void       gen_block(Node *node);
static void       gen_declaration(Node *node);
static void       gen_variable(Node *node);
static void       gen_assign(Node *node);
static void       gen_store(Node *node);
static void       gen_binary(Node *node);
static void       gen_number(Node *node);
static void       gen_string(Node *node);
static void       gen_return(Node *node);
static void       gen_call(Node *node);
static void       gen_asm(Node *node);
static void       gen_syscall(Node *node);
static void       visitor(Node *node);
void              gen(Node *node, List *p);

#endif