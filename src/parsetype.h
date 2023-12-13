#ifndef PARSETYPE_H
#define PARSETYPE_H

#include "parse.h"

void parsetype_peek_class(Node *node);

void parsetype_program(Node *node);
void parsetype_class(Node *node);
void parsetype_method(Ty *class_ty, Node *node);
void parsetype_stmt(Node *node);
void parsetype_if(Node *node);
void parsetype_while(Node *node);
void parsetype_return(Node *node);
void parsetype_decl(Node *node);
void parsetype_assign(Node *node);
void parsetype_expr(node);

Ty  *parsetype_unfold_expr(Node *node);

void parsetype(Node *node);

#endif