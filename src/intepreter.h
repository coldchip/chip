#ifndef INTEPRETER_H
#define INTEPRETER_H

#include "codegen.h"

#define CHIP_VERSION 0x00000001

typedef struct _Object {
	ListNode node;

	char *array;
	int type;
	struct _Slot *varlist;
	int size;

	bool is_marked;
} Object;

typedef struct _Slot {
	bool is_ref;
	union {
		int64_t value;
		double value_float;
		Object *ref;
	};
} Slot;

#define GET_CONST(i) (constants[(int)i])
#define SET_CONST(i, v) (constants[(int)i] = v)

#define SET_VAR_SLOT(k, v) (stack[vp + k] = v)
#define GET_VAR_SLOT(k) (stack[vp + k])

#define TOP_STACK_SLOT() (stack[sp-1])

#define CHECK_STACK() ({ \
	if(sp < 0) {\
		printf("stackunderflow, sp = %li\n", sp); \
		exit(1); \
	} \
	if(sp > 65535 + 1024) {\
		printf("stackoverflow, sp = %li\n", sp); \
		exit(1); \
	} \
})

#define ASSERT(c) ({ \
	if(!c) {\
		printf("assertion failed\n"); \
		exit(1); \
	} \
})

#define DEC_STACK() (sp--, CHECK_STACK())
#define INC_STACK() (sp++, CHECK_STACK())

#define POP_STACK() (DEC_STACK(), stack[sp].is_ref = false, stack[sp].value)
#define PUSH_STACK(v) (stack[sp].value = v, INC_STACK())

#define POP_STACK_DOUBLE() (DEC_STACK(), stack[sp].value_float)
#define PUSH_STACK_DOUBLE(v) (stack[sp].value_float = v, INC_STACK())

#define POP_FRAME() (vp-=512)
#define PUSH_FRAME() (vp+=512)

#define POP_STACK_OBJECT() (DEC_STACK(), ASSERT(stack[sp].is_ref == true), stack[sp].is_ref = false, stack[sp].ref)
#define PUSH_STACK_OBJECT(v) (stack[sp].ref = v, stack[sp].is_ref = true, INC_STACK())

#define POP_STACK_SLOT() ({DEC_STACK(); Slot a = stack[sp]; stack[sp].is_ref = false; a;})
#define PUSH_STACK_SLOT(v) (stack[sp] = v, INC_STACK())


int               load_file(const char *name);
Object           *new_object(int size);
void              gc(Slot *stack, int size);
void              mark(Slot *stack, int size);
void              sweep();
int64_t           eval(int pc);
void              intepreter(const char *input);

#endif