#ifndef VARSCOPE_H
#define VARSCOPE_H

#include "type.h"

typedef struct {
	ListNode node;
	char *name;
	Ty *type;
	int offset;
} VarScope;

void                 varscope_clear();
void                 varscope_push();
void                 varscope_pop();
int                  varscope_size();
VarScope *           varscope_add(char *name, Ty *type);
VarScope            *varscope_get(char *name);

#endif