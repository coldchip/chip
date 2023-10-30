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
	eval.c
*/

char             *strndup(const char *s, size_t len);
char             *strdup(const char *s);
char             *read_file(char *file);

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
	ND_PROGRAM,
	ND_CLASS,
	ND_PARAM,
	ND_ARG,
	ND_METHOD,
	ND_MEMBER,
	ND_NEW,
	ND_BLOCK,
	ND_IF,
	ND_WHILE,
	ND_VARIABLE,
	ND_EXPR,
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

	int length;

	List bodylist;

	Token *token;
} Node;

Node              *new_node(NodeType type, Token *token);
Node              *new_node_binary(NodeType type, Token *token, Node *left, Node *right);

bool               is_class(Token **current);
bool               is_method(Token **current);
bool               is_call(Token **current);
bool               is_assign(Token **current);

static Node       *parse_program(Token **current);
static Node       *parse_class(Token **current);
static Node       *parse_method(Token **current);
Node              *parse_param(Token **current);
Node              *parse_params(Token **current);
Node              *parse_arg(Token **current);
Node              *parse_args(Token **current);
static Node       *parse_stmt(Token **current);
static Node       *parse_expr_stmt(Token **current);

Node              *parse(List *tokens);

/*
	parse_expr.c
*/

Node              *parse_expr(Token **current);
static Node       *parse_relational(Token **current);
static Node       *parse_add_sub(Token **current);
static Node       *parse_mul_div(Token **current);
static Node       *parse_postfix(Token **current);
Node              *parse_primary(Token **current);

/*
	gen.c
*/

typedef enum {
	OP_LOAD_VAR,
	OP_STORE_VAR,
	OP_POP,
	OP_CMPGT,
	OP_CMPLT,
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_MOD,
	OP_LOAD_NUMBER,
	OP_LOAD_CONST,
	OP_LOAD_MEMBER,
	OP_STORE_MEMBER,
	OP_CALL,
	OP_SYSCALL,
	OP_NEW,
	OP_JMPIFT, // pops 2 items from stack and compare, jumps to x if true
	OP_JMP,    // unconditional jump
	OP_RET
} OpType;

typedef struct _Constant {
	ListNode node;
	char *data;
} Constant;

typedef struct _Class {
	ListNode node;
	char *name;
	List method;
} Class;

typedef struct _Method {
	ListNode node;
	char *name;
	List op;
} Method;

typedef struct _Op {
	ListNode node;
	OpType op;
	double left;
	char *left_string;
} Op;

static Class     *emit_class(List *program, char *name);
static Method    *emit_method(Class *class, char *name);
static Op *       emit_op(Method *method, OpType op);
static Op *       emit_op_left(Method *method, OpType op, float left);
static int        emit_constant(List *list, char *data);
static int        emit_op_get_counter(Method *method);
static void       emit_file(List *constants, List *program);

static void       gen_program(Node *node);
static void       gen_class(Node *node);
static void       gen_param(Node *node);
static void       gen_arg(Node *node);
static void       gen_method(Node *node);
static void       gen_if(Node *node);
static void       gen_while(Node *node);
static void       gen_block(Node *node);
static void       gen_variable(Node *node);
static void       gen_member(Node *node);
static void       gen_expr(Node *node);
static void       gen_assign(Node *node);
static void       gen_store(Node *node);
static void       gen_binary(Node *node);
static void       gen_number(Node *node);
static void       gen_string(Node *node);
static void       gen_return(Node *node);
static void       gen_call(Node *node);
static void       gen_syscall(Node *node);
static void       visitor(Node *node);
void              gen(Node *node, List *p);

/*
	intepreter.c
*/

typedef enum {
	TY_NUMBER,
	TY_STRING,
	TY_FUNCTION,
	TY_CUSTOM
} Type;

typedef struct _Object {
	ListNode node;

	Type type;

	Method *method;
	struct _Object *bound;

	char *name;

	double data_number;
	char *data_string;

	List varlist;
} Object;

typedef struct _Var {
	ListNode node;
	char *name;
	Object *object;
} Var;

#define POP_STACK() (sp--, stack[sp])
#define PUSH_STACK(d) (stack[sp++] = d)

static void       load_file(const char *name);
static void       emit_print();
static char      *lookup_constant(int pos);
static Op        *op_at(List *program, int line);
static void       store_var(List *vars, char *name, Object *object);
static Object    *load_var(List *vars, char *name);
static Class     *get_class(char *name);
static Method    *get_method(char *name1, char *name);
static Object    *new_object(Type type, char *name);
Object *          eval(Object *instance, Method *method, List *inject);
void              intepreter(const char *input);

#endif