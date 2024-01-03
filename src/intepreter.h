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
int FIND_OR_INSERT_CONST(char **constants, char *data) {
	for(int i = 0; i < 8192; i++) {
		char *current = GET_CONST(i);
		if(current && strcmp(current, data) == 0) {
			return i;
		}
	}

	return 0;
}

#define SET_VAR_SLOT(k, v) (stack[fp][k] = v)
#define GET_VAR_SLOT(k) (stack[fp][k])

#define TOP_STACK_SLOT() (stack[fp][sp-1])

#define POP_STACK() (sp--, stack[fp][sp].value)
#define PUSH_STACK(v) (stack[fp][sp].value = v, sp++)

#define POP_STACK_DOUBLE() (sp--, stack[fp][sp].value_float)
#define PUSH_STACK_DOUBLE(v) (stack[fp][sp].value_float = v, sp++)

#define POP_FRAME() (fp--)
#define PUSH_FRAME() (fp++)

#define POP_STACK_OBJECT() (sp--, stack[fp][sp].is_ref = false, stack[fp][sp].ref)
#define PUSH_STACK_OBJECT(v) (stack[fp][sp].ref = v, stack[fp][sp].is_ref = true, sp++)

#define POP_STACK_SLOT() ({sp--; Slot a = stack[fp][sp]; stack[fp][sp].is_ref = false; a;})
#define PUSH_STACK_SLOT(v) (stack[fp][sp] = v, sp++)


int               load_file(const char *name);
Object           *new_object(int size);
void              gc(Slot *stack, int size);
void              mark(Slot *stack, int size);
void              sweep();
int64_t           eval(int pc);
void              intepreter(const char *input);

#endif