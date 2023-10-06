#include <string.h>
#include "chip.h"

Var *get_var(List* varlist, char *name) {
	for(ListNode *v = list_begin(varlist); v != list_end(varlist); v = list_next(v)) {
		Var *var = (Var*)v;
		if(strcmp(var->name, name) == 0) {
			return var;
		}
	}
	return NULL;
}

int total_var_size(List* varlist) {
	int size = 0;
	for(ListNode *v = list_begin(varlist); v != list_end(varlist); v = list_next(v)) {
		Var *var = (Var*)v;
		size += var->size;
	}
	return size;
}

int var_pos(List* varlist, char *name) {
	int size = 0;
	for(ListNode *v = list_begin(varlist); v != list_end(varlist); v = list_next(v)) {
		Var *var = (Var*)v;

		if(strcmp(var->name, name) == 0) {
			break;
		}

		size += var->size;
	}
	return size;
}