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

Ty *type_get_method(Ty *ty, char *name) {
	for(ListNode *i = list_begin(&ty->methods); i != list_end(&ty->methods); i = list_next(i)) {
		Ty *method = (Ty*)i;

		if(strcmp(method->name, name) == 0) {
			return method;
		}
	}
	return NULL;
}

void type_insert(char *name) {
	Ty *ty = malloc(sizeof(Ty));
	ty->type = TYPE_CLASS;
	ty->name = strdup(name);
	list_clear(&ty->methods);

	list_insert(list_end(&types), ty);
}

void insert_method(char *name) {
	Ty *ty = (Ty*)list_back(&types);

	Ty *method = malloc(sizeof(Ty));
	method->type = 0;
	method->name = strdup(name);

	list_insert(list_end(&ty->methods), method);
}