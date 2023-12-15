#ifndef GEN_H
#define GEN_H

#include "parse.h"
#include <stdint.h>

typedef enum {
	OP_LOAD,
	OP_STORE,
	OP_POP,
	OP_CMPEQ,
	OP_CMPGT,
	OP_CMPLT,
	/* integer arithmetric operations */
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	/* floating point arithmetric operations */
	OP_FADD,
	OP_FSUB,
	OP_FMUL,
	OP_FDIV,
	OP_MOD,
	OP_OR,

	OP_DUP,

	OP_PUSH,
	OP_LOAD_CONST,
	OP_LOAD_FIELD,
	OP_STORE_FIELD,
	OP_CALL,
	OP_SYSCALL,
	OP_NEWO,
	OP_NEWARRAY,
	OP_LOAD_ARRAY,
	OP_STORE_ARRAY,
	OP_JE, // pops 2 items from stack and compare, jumps to x if true
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
	int index;
	List method;
} Class;


typedef struct _Method {
	ListNode node;
	char *name;
	int index;
	List op;

	struct _Op **codes;
	int code_count;

	Modifier modifier;
} Method;

typedef struct {
	TyMethod *method;
	char name[256];
	int line;
} LabelEntry;

typedef struct _Op {
	OpType op;
	uint64_t left;
	char *left_label;
	bool has_left;
} Op;

static int        rand_string();

LabelEntry        emit_get_label(const char *name);
LabelEntry        emit_label(const char *name);
static Op *       emit_op(OpType op);
static Op *       emit_op_left(OpType op, uint64_t left);
static Op        *emit_op_left_label(OpType op, const char *left);
static int        emit_constant(List *list, char *data, bool obfuscated);
static int        emit_op_get_counter();
static void       emit_file(List *constants);

static void       gen_program(Node *node);
static void       gen_class(Node *node);
static int        gen_param(Node *node);
static int        gen_arg(Node *node);
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
static void       gen_decl(Node *node);
static void       gen_assign(Node *node);
static void       gen_store(Node *node);
static void       gen_binary(Node *node);
static void       gen_char(Node *node);
static void       gen_number(Node *node);
static void       gen_float(Node *node);
static void       gen_string(Node *node);
static void       gen_return(Node *node);
static void       gen_call(Node *node);
static void       gen_syscall(Node *node);
static void       gen_visitor(Node *node);
void              gen(Node *node);

#endif