#ifndef OPTIMIZE_H
#define OPTIMIZE_H

#include "codegen.h"

void optimize(Op **codes, int code_count);
void optimize_shortcut(Op **codes, int code_count);
void optimize_deadcode(Op **codes, int code_count);

#endif