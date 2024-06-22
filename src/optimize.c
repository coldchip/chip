#include <stdio.h>
#include <stdlib.h>
#include "optimize.h"
#include "codegen.h"

void optimize(Op **codes, int code_count, Label *labels, int label_count) {
	optimize_shortcut(codes, code_count, labels, label_count);
	//optimize_deadcode(codes, code_count);
	optimize_graph(codes, code_count, labels, label_count);
}

void optimize_shortcut(Op **codes, int code_count, Label *labels, int label_count) {
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
			Label label = emit_get_label(current->label);
			int line = (label.line);
			optimize_deadcode_recursive(codes, line, code_count, nopes, level + 1);
		}
		if(current->op == OP_JE) {
			Label label = emit_get_label(current->label);
			int line = (label.line);
			optimize_deadcode_recursive(codes, line, code_count, nopes, level + 1);
		}
		// if(current->op == OP_JMP) {
		// 	Label label = emit_get_label(current->label);
		// 	int line = (label.line);
		// 	optimize_deadcode_recursive(codes, line, code_count, nopes, level + 1);
		// }
		if(current->op == OP_RET) {
			return;
		}
	}
}

void optimize_deadcode(Op **codes, int code_count, Label *labels, int label_count) {
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

void optimize_graph(Op **codes, int code_count, Label *labels, int label_count) {
	List graphs;
	list_clear(&graphs);

	graph_t *graph = NULL;

	for(int i = 0; i < code_count; i++) {
		for(int j = 0; j < label_count; ++j) {
			if((labels[j].line) == i) {
				graph = malloc(sizeof(graph_t));
				graph->label = labels[j];
				graph->code_count = 0;
				list_insert(list_end(&graphs), graph);
				break;
			}
		}
		graph->codes[graph->code_count++] = codes[i];
	}

	// eliminate dead code from JMP and RET
	for(ListNode *g = list_begin(&graphs); g != list_end(&graphs); g = list_next(g)) {
		graph_t *graph = (graph_t*)g;

		bool has_ended = false;

		for(int i = 0; i < graph->code_count; i++) {
			Op *ins = graph->codes[i];

			if((ins->op == OP_JMP || ins->op == OP_RET) && has_ended == false) {
				printf("ended\n");
				has_ended = true;
				continue;
			}

			if(has_ended == true) {
				printf("DEL %s\n", op_display[ins->op]);
				graph->codes[i] = NULL;
			}
		}
	}

	printf("graph size: %li\n", list_size(&graphs));
}