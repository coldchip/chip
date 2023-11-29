#ifndef GEN_H
#define GEN_H

#include "parse.h"

typedef enum {
	OP_LOAD_VAR,
	OP_STORE_VAR,
	OP_POP,
	OP_CMPEQ,
	OP_CMPGT,
	OP_CMPLT,
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_MOD,
	OP_OR,
	OP_LOAD_NUMBER,
	OP_LOAD_CONST,
	OP_LOAD_MEMBER,
	OP_STORE_MEMBER,
	OP_CALL,
	OP_SYSCALL,
	OP_NEW,
	OP_NEWARRAY,
	OP_LOAD_ARRAY,
	OP_STORE_ARRAY,
	OP_JMPIFT, // pops 2 items from stack and compare, jumps to x if true
	OP_JMP,    // unconditional jump
	OP_RET
} OpType;

typedef struct _Constant {
	ListNode node;
	char *data;
	bool obfuscated;
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

	struct _Op **codes;
	int code_count;

	Modifier modifier;
} Method;

typedef struct _Op {
	ListNode node;
	OpType op;
	double left;
} Op;

static char      *rand_string(char *str, size_t size);

static Class     *emit_class(List *program, char *name);
static Method    *emit_method(Class *class, char *name);
static Op *       emit_op(Method *method, OpType op);
static Op *       emit_op_left(Method *method, OpType op, float left);
static int        emit_constant(List *list, char *data, bool obfuscated);
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
static void       gen_new(Node *node);
static void       gen_new_array(Node *node);
static void       gen_array_member(Node *node);
static void       gen_expr(Node *node);
static void       gen_declaration(Node *node);
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

#endif