#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "eval.h"

List varlist;

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

static void store_var(char *name, double value) {
	if(load_var(name, NULL)) {
		for(ListNode *i = list_begin(&varlist); i != list_end(&varlist); i = list_next(i)) {
			Var *var = (Var*)i;
			if(strcmp(name, var->name) == 0) {
				var->value = value;
			}
		}
	} else {
		Var *var = malloc(sizeof(Var));
		var->name = strdup(name);
		var->value = value;
		list_insert(list_end(&varlist), var);
	}
}

static bool load_var(char *name, double *value) {
	for(ListNode *i = list_begin(&varlist); i != list_end(&varlist); i = list_next(i)) {
		Var *var = (Var*)i;
		if(strcmp(name, var->name) == 0) {
			if(value) {
				*value = var->value;
			}
			return true;
		}
	}

	return false;
}

void run(List *program) {
	list_clear(&varlist);
	
	Op *current = (Op*)list_begin(program);

	double stack[8192];
	int sp = 0;

	while(current != (Op*)list_end(program)) {
		switch(current->op) {
			case OP_LOAD: {
				double result = 0;
				if(load_var(current->left_string, &result)) {
					stack[sp] = result;
					sp++;
				} else {
					printf("VM:: Unable to load variable %s as it is not found\n", current->left_string);
					exit(1);
				}
			}
			break;
			case OP_STORE: {
				sp--;
				double v1 = stack[sp];
				store_var(current->left_string, v1);
			}
			break;
			case OP_PUSH: {
				stack[sp] = current->left;
				sp++;
			}
			break;
			case OP_CMPGT: {
				sp--;
				double v1 = stack[sp];
				sp--;
				double v2 = stack[sp];

				double v3 = v2 > v1;
				stack[sp] = v3;
				sp++;
			}
			break;
			case OP_CMPLT: {
				sp--;
				double v1 = stack[sp];
				sp--;
				double v2 = stack[sp];

				double v3 = v2 < v1;
				stack[sp] = v3;
				sp++;
			}
			break;
			case OP_ADD: {
				sp--;
				double v1 = stack[sp];
				sp--;
				double v2 = stack[sp];

				double v3 = v2 + v1;
				stack[sp] = v3;
				sp++;
			}
			break;
			case OP_SUB: {
				sp--;
				double v1 = stack[sp];
				sp--;
				double v2 = stack[sp];

				double v3 = v2 - v1;
				stack[sp] = v3;
				sp++;
			}
			break;
			case OP_MUL: {
				sp--;
				double v1 = stack[sp];
				sp--;
				double v2 = stack[sp];

				double v3 = v2 * v1;
				stack[sp] = v3;
				sp++;
			}
			break;
			case OP_DIV: {
				sp--;
				double v1 = stack[sp];
				sp--;
				double v2 = stack[sp];

				double v3 = v2 / v1;
				stack[sp] = v3;
				sp++;
			}
			break;
			case OP_CALL: {
				sp--;
				double v1 = stack[sp];
				if(strcmp(current->left_string, "sin") == 0) {
					double v2 = sin(v1);
					stack[sp] = v2;
					sp++;
				} else {
					printf("VM:: unknown function call %s\n", current->left_string);
					exit(1);
				}

			}
			break;
			case OP_JMPIFT: {
				sp--;
				double v1 = stack[sp];
				sp--;
				double v2 = stack[sp];

				if(v2 == v1) {
					Op *jmp_to = op_at(program, current->left);
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
				Op *jmp_to = op_at(program, current->left);
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
		printf("%s\t%f\n", var->name, var->value);
	}

	printf("\n\n-----------------\n");

	printf("result: %f sp: %i\n", stack[0], sp);
}