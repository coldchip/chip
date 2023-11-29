#ifndef TYPE_H
#define TYPE_H

#include "list.h"

typedef enum DataType {
	TYPE_UNKNOWN,
	TYPE_CLASS,
	TYPE_INT,
	TYPE_FLOAT
} DataType;

typedef struct {
	ListNode node;
	DataType type;
	char *name;
	List methods;
} Ty;

void                 type_clear();
Ty                  *type_current_class();
Ty                  *type_get_class(char *name);
Ty                  *type_get_method(Ty *ty, char *name);
void                 type_insert(char *name);
void                 insert_method(char *name);

#endif