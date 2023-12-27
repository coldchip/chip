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
	uint64_t *varlist;

	int refs;
	int gc_refs;
} Object;

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

#define TOP_STACK() (stack[fp][sp-1])
#define POP_STACK() (sp--, stack[fp][sp])
#define PUSH_STACK(d) (stack[fp][sp++] = d)
#define POP_FRAME() (fp--)
#define PUSH_FRAME() (fp++)
#define POP_STACK_OBJECT() (sp--, *(Object **)&stack[fp][sp])
#define PUSH_STACK_OBJECT(d) (*(Object **)&stack[fp][sp++] = d)


int               load_file(const char *name);
void              store_var(double *vars, int index, Object *object);
double            load_var(double *vars, int index);
Object           *new_object(int size);
int64_t           eval(int pc);
void              intepreter(const char *input);

#endif