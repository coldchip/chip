#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "parse.h"

void         semantic_firstpass_class(Node *node);
void         semantic_firstpass_method(Node *node);

void         semantic_program(Node *node);
void         semantic_class(Node *node);
char        *semantic_param_signature(Node *node);
char        *semantic_arg_signature(Node *node);
void         semantic_param(Node *node);
void         semantic_arg(Node *node);
void         semantic_method(Node *node);
void         semantic_stmt(Node *node);
void         semantic_if(Node *node);
void         semantic_while(Node *node);
void         semantic_for(Node *node);
void         semantic_return(Node *node);
void         semantic_decl(Node *node);
void         semantic_assign(Node *node);
void         semantic_expr(Node *node);

Node        *semantic_walk_expr(Node *node);

void         semantic(Node *node);

#endif