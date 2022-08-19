#include <stdio.h>
#include <string.h>
#include <math.h>
#include "eval.h"

void run(List *program) {
	
	
	Op *current = (Op*)list_begin(program);

	double stack[8192];
	int sp = 0;

	while(current != (Op*)list_end(program)) {
		switch(current->op) {
			case OP_PUSH: {
				stack[sp] = current->left;
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

	printf("result: %f sp: %i\n", stack[0], sp);
}