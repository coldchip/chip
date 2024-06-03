#ifndef GEN_H
#define GEN_H

#include "parse.h"
#include <stdint.h>
#include <arpa/inet.h>

#define LIST_OF_OPS \
	DEFINE_OP(OP_NOP, "nop", false) \
	DEFINE_OP(OP_LOAD, "load", true) \
	DEFINE_OP(OP_LOAD_0, "load_0", false) \
	DEFINE_OP(OP_LOAD_1, "load_1", false) \
	DEFINE_OP(OP_LOAD_2, "load_2", false) \
	DEFINE_OP(OP_LOAD_3, "load_3", false) \
	DEFINE_OP(OP_LOAD_4, "load_4", false) \
	DEFINE_OP(OP_LOAD_5, "load_5", false) \
	DEFINE_OP(OP_STORE, "store", true) \
	DEFINE_OP(OP_STORE_0, "store_0", false) \
	DEFINE_OP(OP_STORE_1, "store_1", false) \
	DEFINE_OP(OP_STORE_2, "store_2", false) \
	DEFINE_OP(OP_STORE_3, "store_3", false) \
	DEFINE_OP(OP_STORE_4, "store_4", false) \
	DEFINE_OP(OP_STORE_5, "store_5", false) \
	DEFINE_OP(OP_CMPEQ, "cmpeq", false) \
	DEFINE_OP(OP_CMPGT, "cmpgt", false) \
	DEFINE_OP(OP_CMPLT, "cmplt", false) \
	DEFINE_OP(OP_SHR, "shr", false) \
	DEFINE_OP(OP_SHL, "shl", false) \
	DEFINE_OP(OP_ADD, "add", false) \
	DEFINE_OP(OP_SUB, "sub", false) \
	DEFINE_OP(OP_MUL, "mul", false) \
	DEFINE_OP(OP_DIV, "div", false) \
	DEFINE_OP(OP_NEG, "neg", false) \
	DEFINE_OP(OP_MOD, "mod", false) \
	DEFINE_OP(OP_NOT, "not", false) \
	DEFINE_OP(OP_OR, "or", false) \
	DEFINE_OP(OP_XOR, "xor", false) \
	DEFINE_OP(OP_AND, "and", false) \
	DEFINE_OP(OP_FADD, "fadd", false) \
	DEFINE_OP(OP_FSUB, "fsub", false) \
	DEFINE_OP(OP_FMUL, "fmul", false) \
	DEFINE_OP(OP_FDIV, "fdiv", false) \
	DEFINE_OP(OP_FNEG, "fneg", false) \
	DEFINE_OP(OP_FMOD, "fmod", false) \
	DEFINE_OP(OP_I2F, "i2f", false) \
	DEFINE_OP(OP_DUP, "dup", false) \
	DEFINE_OP(OP_PUSH,  "push", true) \
	DEFINE_OP(OP_PUSH_0,  "push_0", false) \
	DEFINE_OP(OP_PUSH_1,  "push_1", false) \
	DEFINE_OP(OP_PUSH_2,  "push_2", false) \
	DEFINE_OP(OP_PUSH_3,  "push_3", false) \
	DEFINE_OP(OP_PUSH_4,  "push_4", false) \
	DEFINE_OP(OP_PUSH_5,  "push_5", false) \
	DEFINE_OP(OP_POP, "pop", false) \
	DEFINE_OP(OP_LOAD_CONST, "loadconst", true) \
	DEFINE_OP(OP_LOAD_FIELD, "loadfield", true) \
	DEFINE_OP(OP_STORE_FIELD, "storefield", true) \
	DEFINE_OP(OP_CALL, "call", true) \
	DEFINE_OP(OP_SYSCALL, "syscall", false) \
	DEFINE_OP(OP_ALLOC, "alloc", true) \
	DEFINE_OP(OP_NEW_ARRAY, "newarr", true) \
	DEFINE_OP(OP_LOAD_ARRAY, "loadarr", false) \
	DEFINE_OP(OP_STORE_ARRAY, "storearr", false) \
	DEFINE_OP(OP_JE, "je", true) \
	DEFINE_OP(OP_JMP, "jmp", true) \
	DEFINE_OP(OP_RET, "ret", false) \
	DEFINE_OP(OP_HALT, "halt", false)

/* ops */
#define DEFINE_OP(name, display, left) name,
typedef enum {
	LIST_OF_OPS
} OpType;
#undef DEFINE_OP

/* mnemonic names for ops */
#define DEFINE_OP(name, display, left) display,
static char *op_display[] = {
	LIST_OF_OPS
};
#undef DEFINE_OP

/* ops that has a left oprand */
#define DEFINE_OP(name, display, left) left,
static bool op_size[] = {
	LIST_OF_OPS
};
#undef DEFINE_OP

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
static void       emit_file(const char *file);

uint8_t           closest_container_size(int64_t number);

static void       gen_program(Node *node);
static void       gen_import(Node *node);
static void       gen_class(Node *node);
static int        gen_param(Node *node);
static int        gen_arg(Node *node);
static void       gen_method(Node *node);
static void       gen_if(Node *node);
static void       gen_while(Node *node);
static void       gen_for(Node *node);
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
void              gen(Node *node, const char *file);

#endif