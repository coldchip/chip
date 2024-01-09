#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "type.h"
#include "chip.h"

List types;

void type_clear() {
	list_clear(&types);

	/* built-in types */
	Ty *int_type = type_insert("int", 8);
	insert_variable(int_type, "count", int_type);
	Ty *char_type = type_insert("char", 8);
	insert_variable(char_type, "count", int_type);
	type_insert("float", 8);
	type_insert("void", 8);
}

bool type_compatible(Ty *from, Ty *to) {
	if(from == to) {
		return true;
	}
	if(type_is_primitive(from) && type_is_primitive(to) && type_get_common(from, to) == to) {
		return true;
	}

	return false;
}

bool type_is_primitive(Ty *type) {
	return type == type_get("float") || type == type_get("int") || type == type_get("char");
}

Ty *type_get_common(Ty *left, Ty *right) {
	Ty *ty_float = type_get("float");
	Ty *ty_int   = type_get("int");
	Ty *ty_char  = type_get("char");

	if(left == ty_float || right == ty_float) {
		return ty_float;
	}
	if(left == ty_int || right == ty_int) {
		return ty_int;
	}
	if(left == ty_char || right == ty_char) {
		return ty_char;
	}
	return NULL;
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

TyMethod *insert_method(Ty *class, char *name, char *signature, Ty *type) {
	TyMethod *method = malloc(sizeof(TyMethod));
	method->type = type;
	method->name = strdup(name);
	method->signature = strdup(signature);

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

TyMethod *type_get_method(Ty *class, char *name, char *signature) {
	for(ListNode *i = list_begin(&class->methods); i != list_end(&class->methods); i = list_next(i)) {
		TyMethod *method = (TyMethod*)i;

		if(strcmp(method->name, name) == 0 && strcmp(method->signature, signature) == 0) {
			return method;
		}
	}
	return NULL;
}