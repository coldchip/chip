#include <stdlib.h>
#include <string.h>
#include "type.h"
#include "chip.h"

List types;

void type_clear() {
	list_clear(&types);
}

Ty *type_current_class() {
	return (Ty*)list_back(&types);
}

Ty *type_insert(char *name, int size) {
	Ty *type = malloc(sizeof(Ty));
	type->name = strdup(name);
	type->size = size;
	list_clear(&type->variables);
	list_clear(&type->methods);

	list_insert(list_end(&types), type);

	return type;
}

Ty *type_get(char *name) {
	for(ListNode *i = list_begin(&types); i != list_end(&types); i = list_next(i)) {
		Ty *ty = (Ty*)i;

		if(strcmp(ty->name, name) == 0) {
			return ty;
		}
	}
	return NULL;
}

int type_size(Ty *class) {
	return (int)list_size(&class->variables);
}

TyVariable *insert_variable(Ty *class, char *name, Ty *type) {
	TyVariable *variable = malloc(sizeof(TyMethod));
	variable->type = type;
	variable->name = strdup(name);
	variable->offset = type_size(class);

	list_insert(list_end(&class->variables), variable);
	return variable;
}

TyMethod *insert_method(Ty *class, char *name, Ty **args, int args_count, Ty *type) {
	TyMethod *method = malloc(sizeof(TyMethod));
	method->type = type;
	method->name = strdup(name);

	list_insert(list_end(&class->methods), method);
	return method;
}

TyVariable *type_get_variable(Ty *class, char *name) {
	for(ListNode *i = list_begin(&class->variables); i != list_end(&class->variables); i = list_next(i)) {
		TyVariable *variable = (TyVariable*)i;

		if(strcmp(variable->name, name) == 0) {
			return variable;
		}
	}
	return NULL;
}

TyMethod *type_get_method(Ty *class, char *name) {
	for(ListNode *i = list_begin(&class->methods); i != list_end(&class->methods); i = list_next(i)) {
		TyMethod *method = (TyMethod*)i;

		if(strcmp(method->name, name) == 0) {
			return method;
		}
	}
	return NULL;
}