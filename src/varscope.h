#ifndef VARSCOPE_H
#define VARSCOPE_H

#include "type.h"

typedef struct {
	ListNode node;
	char *name;
	Ty *type;
	int offset;
} Var;

void                 varscope_clear();
void                 varscope_push();
void                 varscope_pop();
int                  varscope_size();
Var *                varscope_add(char *name, Ty *type);
Var                 *varscope_get(char *name);

#endif