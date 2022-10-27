#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "eval.h"

static List *program;

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

static void store_var(List *vars, char *name, Object *object) {
	if(load_var(vars, name)) {
		for(ListNode *i = list_begin(vars); i != list_end(vars); i = list_next(i)) {
			Var *var = (Var*)i;
			if(strcmp(name, var->name) == 0) {
				var->object = object;
			}
		}
	} else {
		Var *var = malloc(sizeof(Var));
		var->name = strdup(name);
		var->object = object;
		list_insert(list_end(vars), var);
	}
}

static Object *load_var(List *vars, char *name) {
	for(ListNode *i = list_begin(vars); i != list_end(vars); i = list_next(i)) {
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

static Method *get_method(char *name1, char *name2) {
	Class *class = get_class(name1);

	if(class) {
		for(ListNode *i = list_begin(&class->method); i != list_end(&class->method); i = list_next(i)) {
			Method *method = (Method*)i;
			if(strcmp(name2, method->name) == 0) {
				return method;
			}
		}
	}
	return NULL;
}

Object *new_object(Type type, char *name) {
	Object *o = malloc(sizeof(Object));
	o->type = type;
	o->name = name;

	list_clear(&o->vars);

	if(type == TY_CUSTOM) {
		Class *skeleton = get_class(name);
		if(skeleton) {
			for(ListNode *i = list_begin(&skeleton->method); i != list_end(&skeleton->method); i = list_next(i)) {
				Method *method = (Method*)i;
				Object *var = new_object(TY_FUNCTION, method->name);
				var->bound = o;
				var->method = method;

				store_var(&o->vars, method->name, var);
			}
		} else {
			printf("Error, class %s not defined\n", name);
			exit(1);
		}
	}

	if(type == TY_STRING) {
		Object *l = new_object(TY_NUMBER, "Number");
		l->data_number = 666;

		Var *var = malloc(sizeof(Var));
		var->name = strdup("length");
		var->object = l;

		list_insert(list_end(&o->vars), var);
	}

	return o;
}

void run(List *p) {
	program = p;

	Method *method_main = get_method("Main", "main");
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
	List varlist;
	list_clear(&varlist);

	if(instance != NULL) {
		store_var(&varlist, "this", instance);
	}
	
	Op *current = (Op*)list_begin(&method->op);

	Object *stack[8192];
	int sp = 0;

	while(current != (Op*)list_end(&method->op)) {
		switch(current->op) {
			case OP_LOAD_VAR: {
				Object *o = load_var(&varlist, current->left_string);
				if(o) {
					stack[sp] = o;
					sp++;
				} else {
					printf("VM:: Unable to load variable %s as it is not found\n", current->left_string);
					exit(1);
				}
			}
			break;
			case OP_STORE_VAR: {
				sp--;
				Object *v1 = stack[sp];
				store_var(&varlist, current->left_string, v1);
			}
			break;
			case OP_LOAD_NUMBER: {
				Object *o = new_object(TY_NUMBER, "Number");
				o->data_number = current->left;
				stack[sp] = o;
				sp++;
			}
			break;
			case OP_LOAD_CONST: {
				Object *o = new_object(TY_STRING, "String");
				o->data_string = strdup(current->left_string);
				stack[sp] = o;
				sp++;
			}
			break;
			case OP_LOAD_MEMBER: {
				sp--;
				Object *instance = stack[sp];

				printf("LOAD: %s\n", current->left_string);

				Object *v2 = load_var(&instance->vars, current->left_string);
				printf("%p\n", v2);
				if(v2) {
					stack[sp] = v2;
					sp++;
				} else {
					printf("Unknown variable member %s\n", current->left_string);
					exit(1);
				}
			}
			break;
			case OP_STORE_MEMBER: {
				sp--;
				Object *v1 = stack[sp];

				sp--;
				Object *v2 = stack[sp];

				store_var(&v1->vars, current->left_string, v2);
			}
			break;
			case OP_CMPGT: {
				sp--;
				Object *v1 = stack[sp];
				sp--;
				Object *v2 = stack[sp];

				Object *v3 = new_object(TY_NUMBER, "Number");
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

				Object *v3 = new_object(TY_NUMBER, "Number");
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
					Object *v3 = new_object(TY_NUMBER, "Number");
					v3->data_number = v2->data_number + v1->data_number;
					stack[sp] = v3;
					sp++;
				} else if(v1->type == TY_STRING && v2->type == TY_STRING) {
					Object *v3 = new_object(TY_STRING, "String");

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

				Object *v3 = new_object(TY_NUMBER, "Number");
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

				Object *v3 = new_object(TY_NUMBER, "Number");
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

				Object *v3 = new_object(TY_NUMBER, "Number");
				v3->data_number = v2->data_number / v1->data_number;
				stack[sp] = v3;
				sp++;
			}
			break;
			case OP_CALL: {
				sp--;
				Object *instance = stack[sp];

				if(instance->type == TY_FUNCTION) {
					eval(instance->bound, instance->method);

					// stack[sp] = v2;
					// sp++;
				} else {
					printf("VM:: unknown function call %s\n", instance->data_string);
					exit(1);
				}
			}
			break;
			case OP_NEW: {
				sp--;
				Object *name = stack[sp];

				Class *class = get_class(name->data_string);
				if(class) {
					Object *instance = new_object(TY_CUSTOM, strdup(name->data_string));

					
					Method *constructor = get_method(strdup(name->data_string), "constructor");
					if(constructor) {
						eval(instance, constructor);
					}
					

					stack[sp] = instance;
					sp++;
				} else {
					printf("VM:: cannot clone class %s\n", name->data_string);
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
			case TY_FUNCTION: {
				printf("%s\t#Function@%p\t\n", var->name, var->object);
			}
			break;
			case TY_CUSTOM: {
				printf("%s\t%s@%p\t\n", var->name, var->object->name, var->object);
			}
			break;
		}
	}

	printf("\n\n-----------------\n");

	printf("result: %f sp: %i\n", stack[0], sp);
}