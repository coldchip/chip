#include "optimize.h"
#include "codegen.h"

void optimize_peephole(Op **codes, int code_count) {
	for(int i = 0; i < code_count; ++i) {
		Op *current = codes[i];

		if(current->left >= 0 && current->left <= 5) {
			switch(current->op) {
				case OP_LOAD: {
					current->op = OP_LOAD_0 + current->left;
				}
				break;
				case OP_STORE: {
					current->op = OP_STORE_0 + current->left;
				}
				break;
				case OP_PUSH: {
					current->op = OP_PUSH_0 + current->left;
				}
				break;
			}
		}

		// if(i > 0) {
		// 	Op *previous = codes[i - 1];
		// 	if(
		// 		(
		// 			previous->op == OP_LOAD
		// 		) 
		// 		&& 
		// 		previous->op == current->op) {
		// 		current->op = OP_DUP;
		// 	}
		// }
	}
}
