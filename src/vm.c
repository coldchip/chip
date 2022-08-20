#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "eval.h"

List varlist;

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
					printf("unknown function call %s\n", current->left_string);
				}

			}
			break;
		}

		current = (Op*)list_next((ListNode*)current);
	}

	for(ListNode *i = list_begin(&varlist); i != list_end(&varlist); i = list_next(i)) {
		Var *var = (Var*)i;
		printf("%s %f\n", var->name, var->value);
	}

	printf("result: %f sp: %i\n", stack[0], sp);
}