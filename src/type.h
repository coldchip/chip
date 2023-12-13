#ifndef TYPE_H
#define TYPE_H

#include "list.h"

typedef struct {
	ListNode node;
	char *name;
	List methods;
} Ty;

typedef struct {
	ListNode node;
	Ty *type;
	char *name;
} TyMethod;

void                 type_clear();
Ty                  *type_current_class();
Ty                  *type_get(char *name);
TyMethod            *type_get_method(Ty *ty, char *name);
Ty *                 type_insert(char *name);
TyMethod            *insert_method(Ty *class, char *name, Ty **args, int args_count, Ty *type);

#endif