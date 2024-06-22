#ifndef OPTIMIZE_H
#define OPTIMIZE_H

#include "list.h"
#include "codegen.h"

typedef struct {
	ListNode node;
	Label label;
	Op *codes[8192];
	int code_count;
} graph_t;

void            optimize(Op **codes, int code_count, Label *labels, int label_count);
void            optimize_shortcut(Op **codes, int code_count, Label *labels, int label_count);
void            optimize_deadcode(Op **codes, int code_count, Label *labels, int label_count);
void            optimize_graph(Op **codes, int code_count, Label *labels, int label_count);

#endif