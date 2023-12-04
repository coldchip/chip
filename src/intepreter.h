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

	double data_number;
	char *data_string;
	char *array;

	uint64_t varlist[2048];

	int refs;
	int gc_refs;
} Object;

typedef struct _Var {
	ListNode node;
	int index;
	Object *object;
} Var;

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

#define DECREF(o) (decref_object(o))
#define INCREF(o) (incref_object(o))
#define POP_STACK() (sp--, stack[sp])
#define PUSH_STACK(d) (stack[sp++] = d)

#define POP_STACK_2() (sp2--, stack2[sp2])
#define PUSH_STACK_2(d) (stack2[sp2++] = d)
#define POP_STACK_OBJECT_2() (sp2--, *(Object **)&stack2[sp2])
#define PUSH_STACK_OBJECT_2(d) (*(Object **)&stack2[sp2++] = d)


void              load_file(const char *name);
void              emit_print();
Op               *op_at(List *program, int line);
void              store_var(double *vars, int index, Object *object);
double            load_var(double *vars, int index);
Class            *get_class(int index);
Method           *get_method(int name1, int name2);
Object           *new_object(Type type, int index);
uint64_t          eval(Object *instance, Method *method, uint64_t *args, int args_length);
void              intepreter(const char *input);

#endif