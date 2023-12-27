#ifndef GEN_H
#define GEN_H

#include "parse.h"
#include <stdint.h>

#define LIST_OF_OPS \
	X(OP_NOP, "nop") \
	X(OP_LOAD, "load") \
	X(OP_STORE, "store") \
	X(OP_MOV, "mov") \
	X(OP_POP, "pop") \
	X(OP_CMPEQ, "cmpeq") \
	X(OP_CMPGT, "cmpgt") \
	X(OP_CMPLT, "cmplt") \
	X(OP_ADD, "add") \
	X(OP_SUB, "sub") \
	X(OP_MUL, "mul") \
	X(OP_DIV, "div") \
	X(OP_NEG, "neg") \
	X(OP_FADD, "fadd") \
	X(OP_FSUB, "fsub") \
	X(OP_FMUL, "fmul") \
	X(OP_FDIV, "fdiv") \
	X(OP_MOD, "mod") \
	X(OP_OR, "or") \
	X(OP_DUP, "dup") \
	X(OP_PUSH, "push") \
	X(OP_LOAD_CONST, "loadconst") \
	X(OP_LOAD_FIELD, "loadfield") \
	X(OP_STORE_FIELD, "storefield") \
	X(OP_CALL, "call") \
	X(OP_SYSCALL, "syscall") \
	X(OP_NEWO, "newo") \
	X(OP_NEW_ARRAY, "newarr") \
	X(OP_LOAD_ARRAY, "loadarr") \
	X(OP_STORE_ARRAY, "storearr") \
	X(OP_JE, "je") \
	X(OP_JMP, "jmp") \
	X(OP_RET, "ret")


#define X(name, display) name,
typedef enum {
  LIST_OF_OPS
} OpType;
#undef X

#define X(name, display) display,
static char *op_display[] = {
  LIST_OF_OPS
};
#undef X

typedef struct _Constant {
	ListNode node;
	char *data;
	bool obfuscated;
} Constant;

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

void              emit_label_to_address();
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
static void       gen_neg(Node *node);
static void       gen_not(Node *node);
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