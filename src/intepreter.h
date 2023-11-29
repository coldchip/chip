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

	char *name;

	double data_number;
	char *data_string;
	struct _Object **array;

	List varlist;

	int refs;
	int gc_refs;
} Object;

typedef struct _Var {
	ListNode node;
	char name[256];
	Object *object;
} Var;

#define GET_CONST(i) (constants[(int)i])
#define SET_CONST(i, v) (constants[(int)i] = v)

#define DECREF(o) (decref_object(o))
#define INCREF(o) (incref_object(o))
#define POP_STACK() (sp--, stack[sp])
#define PUSH_STACK(d) (stack[sp++] = d)

void              load_file(const char *name);
void              emit_print();
Op               *op_at(List *program, int line);
void              store_var(List *vars, char *name, Object *object);
Var              *load_var(List *vars, char *name);
Class            *get_class(char *name);
Method           *get_method(char *name1, char *name);
Object           *new_object(Type type, char *name);
Object           *eval(Object *instance, Method *method, Object **args, int args_length);
void              intepreter(const char *input);

#endif