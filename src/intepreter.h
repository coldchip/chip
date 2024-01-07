#ifndef INTEPRETER_H
#define INTEPRETER_H

#include "codegen.h"

typedef struct _Object {
	ListNode node;

	char *array;
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

#define POP_STACK() (sp--, stack[sp].value)
#define PUSH_STACK(v) (stack[sp].value = v, sp++)

#define POP_STACK_DOUBLE() (sp--, stack[sp].value_float)
#define PUSH_STACK_DOUBLE(v) (stack[sp].value_float = v, sp++)

#define POP_FRAME() (vp-=512)
#define PUSH_FRAME() (vp+=512)

#define POP_STACK_OBJECT() (sp--, stack[sp].is_ref = false, stack[sp].ref)
#define PUSH_STACK_OBJECT(v) (stack[sp].ref = v, stack[sp].is_ref = true, sp++)

#define POP_STACK_SLOT() ({sp--; Slot a = stack[sp]; stack[sp].is_ref = false; a;})
#define PUSH_STACK_SLOT(v) (stack[sp] = v, sp++)


int               load_file(const char *name);
Object           *new_object(int size);
void              gc(Slot *stack, int size);
void              mark(Slot *stack, int size);
void              sweep();
int64_t           eval(int pc);
void              intepreter(const char *input);

#endif