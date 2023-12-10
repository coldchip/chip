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

Ty *type_get_class(char *name) {
	for(ListNode *i = list_begin(&types); i != list_end(&types); i = list_next(i)) {
		Ty *ty = (Ty*)i;

		if(strcmp(ty->name, name) == 0) {
			return ty;
		}
	}
	return NULL;
}

TyMethod *type_get_method(Ty *ty, char *name) {
	for(ListNode *i = list_begin(&ty->methods); i != list_end(&ty->methods); i = list_next(i)) {
		TyMethod *method = (TyMethod*)i;

		if(strcmp(method->name, name) == 0) {
			return method;
		}
	}
	return NULL;
}

Ty *type_insert(char *name) {
	Ty *type = malloc(sizeof(Ty));
	type->name = strdup(name);
	list_clear(&type->methods);

	list_insert(list_end(&types), type);

	return type;
}

TyMethod *insert_method(char *name, Ty *type) {
	Ty *ty = (Ty*)list_back(&types);

	TyMethod *method = malloc(sizeof(TyMethod));
	method->type = type;
	method->name = strdup(name);

	list_insert(list_end(&ty->methods), method);
	return method;
}