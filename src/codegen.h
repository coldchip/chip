#ifndef GEN_H
#define GEN_H

#include "parse.h"
#include <stdint.h>
#include <arpa/inet.h>

#define LIST_OF_OPS \
	X(OP_NOP, "nop", false) \
	X(OP_LOAD, "load", true) \
	X(OP_STORE, "store", true) \
	X(OP_CMPEQ, "cmpeq", false) \
	X(OP_CMPGT, "cmpgt", false) \
	X(OP_CMPLT, "cmplt", false) \
	X(OP_ADD, "add", false) \
	X(OP_SUB, "sub", false) \
	X(OP_MUL, "mul", false) \
	X(OP_DIV, "div", false) \
	X(OP_NEG, "neg", false) \
	X(OP_MOD, "mod", false) \
	X(OP_NOT, "not", false) \
	X(OP_OR, "or", false) \
	X(OP_XOR, "xor", false) \
	X(OP_AND, "and", false) \
	X(OP_FADD, "fadd", false) \
	X(OP_FSUB, "fsub", false) \
	X(OP_FMUL, "fmul", false) \
	X(OP_FDIV, "fdiv", false) \
	X(OP_FNEG, "fneg", false) \
	X(OP_FMOD, "fmod", false) \
	X(OP_I2F, "i2f", false) \
	X(OP_DUP, "dup", false) \
	X(OP_PUSH,  "push", true) \
	X(OP_POP, "pop", false) \
	X(OP_LOAD_CONST, "loadconst", true) \
	X(OP_LOAD_FIELD, "loadfield", true) \
	X(OP_STORE_FIELD, "storefield", true) \
	X(OP_CALL, "call", true) \
	X(OP_SYSCALL, "syscall", false) \
	X(OP_NEWO, "newo", true) \
	X(OP_NEW_ARRAY, "newarr", false) \
	X(OP_LOAD_ARRAY, "loadarr", false) \
	X(OP_STORE_ARRAY, "storearr", false) \
	X(OP_JE, "je", true) \
	X(OP_JMP, "jmp", true) \
	X(OP_RET, "ret", false) \
	X(OP_HALT, "halt", false)

/* ops */
#define X(name, display, left) name,
typedef enum {
	LIST_OF_OPS
} OpType;
#undef X

/* mnemonic names for ops */
#define X(name, display, left) display,
static char *op_display[] = {
	LIST_OF_OPS
};
#undef X

/* ops that has a left oprand */
#define X(name, display, left) left,
static bool op_size[] = {
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
	char *label;
	int width;
} Op;

typedef struct __attribute__((__packed__)) {
	char magic[8];
	uint32_t version;
	uint64_t code_size;
	uint64_t const_size;
	uint64_t entry;
} chip_hdr_t;

static int        rand_string();

void              emit_label_to_address();
LabelEntry        emit_get_label(const char *name);
LabelEntry        emit_label(const char *name);
static Op *       emit_op(OpType op);
static Op *       emit_op_left(OpType op, uint64_t left);
static Op        *emit_op_left_label(OpType op, const char *left);
static int        emit_constant(List *list, char *data, bool obfuscated);
static void       emit_file();

uint8_t           closest_container_size(int64_t number);

static void       gen_program(Node *node);
static void       gen_import(Node *node);
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
static void       gen_bitnot(Node *node);
static void       gen_not(Node *node);
static void       gen_cast(Node *node);
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