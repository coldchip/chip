#include <stdio.h>
#include "optimize.h"
#include "codegen.h"

void optimize(Op **codes, int code_count) {
	optimize_shortcut(codes, code_count);
	//optimize_deadcode(codes, code_count);
}

void optimize_shortcut(Op **codes, int code_count) {
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
	}
}

void optimize_deadcode_recursive(Op **codes, int start, int code_count, bool *nopes, int level) {
	for(int i = start; i < code_count; ++i) {
		Op *current = codes[i];
		nopes[i] = true;
		if(current->op == OP_CALL) {
			LabelEntry label = emit_get_label(current->label);
			int line = (label.line);
			optimize_deadcode_recursive(codes, line, code_count, nopes, level + 1);
		}
		if(current->op == OP_JE) {
			LabelEntry label = emit_get_label(current->label);
			int line = (label.line);
			optimize_deadcode_recursive(codes, line, code_count, nopes, level + 1);
		}
		// if(current->op == OP_JMP) {
		// 	LabelEntry label = emit_get_label(current->label);
		// 	int line = (label.line);
		// 	optimize_deadcode_recursive(codes, line, code_count, nopes, level + 1);
		// }
		if(current->op == OP_RET) {
			return;
		}
	}
}

void optimize_deadcode(Op **codes, int code_count) {
	bool nopes[code_count];


	for(int i = 0; i < code_count; i++) {
		nopes[i] = false;
	}


	int level = 0;
	optimize_deadcode_recursive(codes, 0, code_count, nopes, level);

	int deadcodes = 0;
	for(int i = 0; i < code_count; i++) {
		if(nopes[i] == false) {
			deadcodes++;
			codes[i]->op = OP_NOP;
			codes[i]->label = NULL;
		}
	}

	printf("deadcodes optimised: %i\n", deadcodes);
}
