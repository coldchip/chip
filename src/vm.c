#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "eval.h"

static List *program;
static List varlist;

static Op *op_at(List *program, int line) {
	int size = 1;
	ListNode *position;

	for(position = list_begin(program); position != list_end(program); position = list_next(position)) {
		if(size == line) {
			return (Op*)position;
		}
		size++;
	}

	return NULL;
}

static void store_var(char *name, Object *object) {
	if(load_var(name)) {
		for(ListNode *i = list_begin(&varlist); i != list_end(&varlist); i = list_next(i)) {
			Var *var = (Var*)i;
			if(strcmp(name, var->name) == 0) {
				var->object = object;
			}
		}
	} else {
		Var *var = malloc(sizeof(Var));
		var->name = strdup(name);
		var->object = object;
		list_insert(list_end(&varlist), var);
	}
}

static Object *load_var(char *name) {
	for(ListNode *i = list_begin(&varlist); i != list_end(&varlist); i = list_next(i)) {
		Var *var = (Var*)i;
		if(strcmp(name, var->name) == 0) {
			return var->object;
		}
	}
	return NULL;
}

static Class *get_class(char *name) {
	for(ListNode *i = list_begin(program); i != list_end(program); i = list_next(i)) {
		Class *class = (Class*)i;
		if(strcmp(name, class->name) == 0) {
			return class;
		}
	}
	return NULL;
}

static Method *get_method(Class *class, char *name) {
	for(ListNode *i = list_begin(&class->method); i != list_end(&class->method); i = list_next(i)) {
		Method *method = (Method*)i;
		if(strcmp(name, method->name) == 0) {
			return method;
		}
	}
	return NULL;
}

Object *new_object() {
	Object *o = malloc(sizeof(Object));
	return o;
}

void run(List *p) {
	program = p;

	Class *class_main = get_class("Main");
	if(!class_main) {
		printf("entry point class Main not found\n");
		exit(1);
	}

	Method *method_main = get_method(class_main, "main");
	if(!method_main) {
		printf("entry point method main not found\n");
		exit(1);
	}

	eval(NULL, method_main);
}

void eval(Object *instance, Method *method) {
	/*
		assume if instance == NULL then method is static
	*/

	list_clear(&varlist);
	
	Op *current = (Op*)list_begin(&method->op);

	Object *stack[8192];
	int sp = 0;

	while(current != (Op*)list_end(&method->op)) {
		switch(current->op) {
			case OP_LOAD: {
				Object *o = load_var(current->left_string);
				if(o) {
					stack[sp] = o;
					sp++;
				} else {
					printf("VM:: Unable to load variable %s as it is not found\n", current->left_string);
					exit(1);
				}
			}
			break;
			case OP_STORE: {
				sp--;
				Object *v1 = stack[sp];
				store_var(current->left_string, v1);
			}
			break;
			case OP_PUSH: {
				Object *o = new_object();
				o->type = TY_NUMBER;
				o->data_number = current->left;
				stack[sp] = o;
				sp++;
			}
			break;
			case OP_LOAD_CONST: {
				Object *o = new_object();
				o->type = TY_STRING;
				o->data_string = strdup(current->left_string);
				stack[sp] = o;
				sp++;
			}
			break;
			case OP_CMPGT: {
				sp--;
				Object *v1 = stack[sp];
				sp--;
				Object *v2 = stack[sp];

				Object *v3 = new_object();
				v3->data_number = v2->data_number > v1->data_number;
				stack[sp] = v3;
				sp++;
			}
			break;
			case OP_CMPLT: {
				sp--;
				Object *v1 = stack[sp];
				sp--;
				Object *v2 = stack[sp];

				Object *v3 = new_object();
				v3->data_number = v2->data_number < v1->data_number;
				stack[sp] = v3;
				sp++;
			}
			break;
			case OP_ADD: {
				sp--;
				Object *v1 = stack[sp];
				sp--;
				Object *v2 = stack[sp];

				if(v1->type == TY_NUMBER && v2->type == TY_NUMBER) {
					Object *v3 = new_object();
					v3->data_number = v2->data_number + v1->data_number;
					stack[sp] = v3;
					sp++;
				} else if(v1->type == TY_STRING && v2->type == TY_STRING) {
					Object *v3 = new_object();
					v3->type = TY_STRING;

					char *s = malloc(strlen(v1->data_string) + strlen(v2->data_string) + 1);
					strcpy(s, v2->data_string);
					strcat(s, v1->data_string);
					v3->data_string = s;
					stack[sp] = v3;
					sp++;
				}
			}
			break;
			case OP_SUB: {
				sp--;
				Object *v1 = stack[sp];
				sp--;
				Object *v2 = stack[sp];

				Object *v3 = new_object();
				v3->data_number = v2->data_number - v1->data_number;
				stack[sp] = v3;
				sp++;
			}
			break;
			case OP_MUL: {
				sp--;
				Object *v1 = stack[sp];
				sp--;
				Object *v2 = stack[sp];

				Object *v3 = new_object();
				v3->data_number = v2->data_number * v1->data_number;
				stack[sp] = v3;
				sp++;
			}
			break;
			case OP_DIV: {
				sp--;
				Object *v1 = stack[sp];
				sp--;
				Object *v2 = stack[sp];

				Object *v3 = new_object();
				v3->data_number = v2->data_number / v1->data_number;
				stack[sp] = v3;
				sp++;
			}
			break;
			case OP_CALL: {
				sp--;
				Object *v1 = stack[sp];

				Class *class = get_class(v1->data_string);

				if(class) {
					
				} else {
					printf("VM:: unknown function call %s\n", v1->data_string);
					exit(1);
				}

			}
			break;
			case OP_JMPIFT: {
				sp--;
				Object *v1 = stack[sp];
				sp--;
				Object *v2 = stack[sp];

				if(v2->data_number == v1->data_number) {
					Op *jmp_to = op_at(&method->op, current->left);
					if(jmp_to) {
						current = jmp_to;
						continue;
					} else {
						printf("VM:: Unable to jump to op %i\n", (int)current->left);
						exit(1);
					}
				}
			}
			break;
			case OP_JMP: {
				Op *jmp_to = op_at(&method->op, current->left);
				if(jmp_to) {
					current = jmp_to;
					continue;
				} else {
					printf("VM:: Unable to jump to op %i\n", (int)current->left);
					exit(1);
				}
			}
			break;
		}

		current = (Op*)list_next((ListNode*)current);
	}

	printf("\n\nVARLIST\n-----------------\n");

	for(ListNode *i = list_begin(&varlist); i != list_end(&varlist); i = list_next(i)) {
		Var *var = (Var*)i;
		switch(var->object->type) {
			case TY_NUMBER: {
				printf("%s\tNumber@%p\t%f\n", var->name, var->object, var->object->data_number);
			}
			break;
			case TY_STRING: {
				printf("%s\tString@%p\t%s\n", var->name, var->object, var->object->data_string);
			}
			break;
		}
	}

	printf("\n\n-----------------\n");

	printf("result: %f sp: %i\n", stack[0], sp);
}