#ifndef TYPE_H
#define TYPE_H

#include "list.h"

typedef struct {
	ListNode node;
	char *name;
	List variables;
	List methods;
	int size;
} Ty;

typedef struct {
	ListNode node;
	Ty *type;
	char *name;
	int offset;
} TyVariable;

typedef struct {
	ListNode node;
	Ty *type;
	char *name;
} TyMethod;

void                 type_clear();
Ty                  *type_current_class();
Ty *                 type_insert(char *name, int size);
Ty                  *type_get(char *name);
int                  type_size(Ty *class);
TyVariable          *insert_variable(Ty *class, char *name, Ty *type);
TyMethod            *insert_method(Ty *class, char *name, Ty **args, int args_count, Ty *type);
TyVariable          *type_get_variable(Ty *class, char *name);
TyMethod            *type_get_method(Ty *class, char *name);

#endif