#include "optimize.h"
#include "codegen.h"

void optimize_peephole(Op **codes, int code_count) {
	for(int i = 0; i < code_count; ++i) {
		// Op *current = codes[i];

		// if(i > 0) {
		// 	Op *previous = codes[i - 1];
		// 	if(previous->op == OP_STORE && current->op == OP_LOAD && previous->left == current->left) {
		// 		previous->op = OP_NOP;
		// 		current->op  = OP_MOV;
		// 	}
		// }
	}
}
