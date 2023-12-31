#ifndef INTEPRETER_H
#define INTEPRETER_H

#include "codegen.h"

typedef enum {
	TY_FUNCTION,
	TY_VARIABLE,
	TY_ARRAY
} Type;

typedef struct _Object {
	ListNode node;

	Type type;

	int index;

	char *array;
	struct _Slot *varlist;

	int refs;
	int gc_refs;
} Object;

typedef struct _Slot {
	bool is_ref;
	union {
		int64_t value;
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
#define POP_FRAME() (fp--)
#define PUSH_FRAME() (fp++)
#define POP_STACK_OBJECT() (sp--, stack[fp][sp].is_ref = false, stack[fp][sp].ref)
#define PUSH_STACK_OBJECT(v) (stack[fp][sp].ref = v, stack[fp][sp].is_ref = true, sp++)
#define POP_STACK_SLOT() (sp--, stack[fp][sp])
#define PUSH_STACK_SLOT(v) (stack[fp][sp] = v, sp++)


int               load_file(const char *name);
void              store_var(double *vars, int index, Object *object);
double            load_var(double *vars, int index);
Object           *new_object(int size);
int64_t           eval(int pc);
void              intepreter(const char *input);

#endif