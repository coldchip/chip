#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "varscope.h"
#include "chip.h"

List varscope[8192];
int sp = 0;

void varscope_clear() {
	for(int i = 0; i < 8192; i++) {
		list_clear(&varscope[i]);
	}
}

void varscope_push() {
	sp++;
	list_clear(&varscope[sp]);
}

void varscope_pop() {
	list_clear(&varscope[sp]);
	sp--;
}

int varscope_size() {
	return (int)list_size(&varscope[sp]);
}

Var *varscope_add(char *name, Ty *type) {
	Var *var = malloc(sizeof(Var));
	var->name = strdup(name);
	var->type = type;
	var->offset = varscope_size();

	list_insert(list_end(&varscope[sp]), var);

	return var;
}

Var *varscope_get(char *name) {
	for(int i = 0; i <= sp; i++) {
		for(ListNode *v = list_begin(&varscope[i]); v != list_end(&varscope[i]); v = list_next(v)) {
			Var *var = (Var*)v;

			if(strcmp(var->name, name) == 0) {
				return var;
			}
		}
	}
	return NULL;
}