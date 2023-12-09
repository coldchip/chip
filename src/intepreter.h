#ifndef INTEPRETER_H
#define INTEPRETER_H

#include "gen.h"

typedef enum {
	TY_FUNCTION,
	TY_VARIABLE,
	TY_ARRAY
} Type;

typedef struct _Object {
	ListNode node;

	Type type;

	Method *method;
	struct _Object *bound;

	int index;

	char *array;
	uint64_t varlist[2048];

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
}

#define POP_STACK() (sp--, stack[sp])
#define PUSH_STACK(d) (stack[sp++] = d)
#define POP_STACK_OBJECT() (sp--, *(Object **)&stack[sp])
#define PUSH_STACK_OBJECT(d) (*(Object **)&stack[sp++] = d)


void              load_file(const char *name);
void              emit_print();
void              store_var(double *vars, int index, Object *object);
double            load_var(double *vars, int index);
Class            *get_class(int index);
Method           *get_method(int name1, int name2);
Object           *new_object(Type type, int index);
int64_t           eval(Object *instance, Method *method, int64_t *args, int args_length);
void              intepreter(const char *input);

#endif