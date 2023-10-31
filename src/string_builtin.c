#include <string.h>
#include "chip.h"

/* builtin methods for String class */

Object *string_length(Object *instance) {

	int length = strlen(instance->data_string);

	Object *ret = new_object(TY_CUSTOM, "Number");
	ret->data_number = length;

	return ret;
}