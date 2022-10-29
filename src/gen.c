#include <stdio.h>
#include <stdlib.h>
#include "eval.h"

static Class *emit_class(List *program, char *name) {
	Class *c = malloc(sizeof(Class));
	c->name = strdup(name);

	list_clear(&c->method);
	list_insert(list_end(program), c);
	return c;
}

static Method *emit_method(Class *class, char *name) {
	Method *m = malloc(sizeof(Method));
	m->name = strdup(name);

	list_clear(&m->op);
	list_insert(list_end(&class->method), m);
	return m;
}

static Op *emit_op(Method *method, OpType op) {
	Op *ins = malloc(sizeof(Op));
	ins->op = op;
	list_insert(list_end(&method->op), ins);
	return ins;
}

static Op *emit_op_left(Method *method, OpType op, float left) {
	Op *ins = malloc(sizeof(Op));
	ins->op = op;
	ins->left = left;
	list_insert(list_end(&method->op), ins);
	return ins;
}

static Op *emit_op_left_string(Method *method, OpType op, char *left_string) {
	Op *ins = malloc(sizeof(Op));
	ins->op = op;
	ins->left_string = left_string;
	list_insert(list_end(&method->op), ins);
	return ins;
}

static int emit_op_get_counter(Method *program) {
	return list_size(&program->op) + 1;
}

static void emit_print(List *program) {

	for(ListNode *cn = list_begin(program); cn != list_end(program); cn = list_next(cn)) {
		Class *c = (Class*)cn;

		printf("@class %s\n", c->name);

		for(ListNode *mn = list_begin(&c->method); mn != list_end(&c->method); mn = list_next(mn)) {
			Method *m = (Method*)mn;

			int line = 1;
			printf("\t@method %s\n", m->name);
			printf("\tLINE\tOP\tVALUE\n\t--------------------------\n");

			for(ListNode *op = list_begin(&m->op); op != list_end(&m->op); op = list_next(op)) {
				Op *ins = (Op*)op;
				switch(ins->op) {
					case OP_LOAD_VAR: {
						printf("\t%i\tLOAD_VAR\t%s\n", line, ins->left_string);
					}
					break;
					case OP_STORE_VAR: {
						printf("\t%i\tSTORE_VAR\t%s\n", line, ins->left_string);
					}
					break;
					case OP_CMPGT: {
						printf("\t%i\tCMPGT\n", line);
					}
					break;
					case OP_CMPLT: {
						printf("\t%i\tCMPLT\n", line);
					}
					break;
					case OP_ADD: {
						printf("\t%i\tADD\n", line);
					}
					break;
					case OP_SUB: {
						printf("\t%i\tSUB\n", line);
					}
					break;
					case OP_MUL: {
						printf("\t%i\tMUL\n", line);
					}
					break;
					case OP_DIV: {
						printf("\t%i\tDIV\n", line);
					}
					break;
					case OP_MOD: {
						printf("\t%i\tMOD\n", line);
					}
					break;
					case OP_LOAD_NUMBER: {
						printf("\t%i\tLOAD_NUMBER\t%f\n", line, ins->left);
					}
					break;
					case OP_LOAD_CONST: {
						printf("\t%i\tLOAD_CONST\t%s\n", line, ins->left_string);
					}
					break;
					case OP_LOAD_MEMBER: {
						printf("\t%i\tLOAD_MEMBER\t%s\n", line, ins->left_string);
					}
					break;
					case OP_STORE_MEMBER: {
						printf("\t%i\tSTORE_MEMBER\t%s\n", line, ins->left_string);
					}
					break;
					case OP_CALL: {
						printf("\t%i\tCALL\tARGLEN: %i\n", line, (int)ins->left);
					}
					break;
					case OP_NEW: {
						printf("\t%i\tNEW\tARGLEN: %i\n", line, (int)ins->left);
					}
					break;
					case OP_JMPIFT: {
						printf("\t%i\tJMPIFT\t%i\n", line, (int)ins->left);
					}
					break;
					case OP_JMP: {
						printf("\t%i\tJMP\t%i\n", line, (int)ins->left);
					}
					break;
					case OP_RET: {
						printf("\t%i\tRET\t\n", line);
					}
					break;
				}

				line++;
			}
		}
	}
}

static List *program;
static Class *class = NULL;
static Method *method = NULL;

static void gen_program(Node *node) {
	List *list = &node->bodylist;
	while(!list_empty(list)) {
		Node *entry = (Node*)list_remove(list_begin(list));
		visitor(entry);
	}
}

static void gen_class(Node *node) {
	class = emit_class(program, strndup(node->token->data, node->token->length));

	List *list = &node->bodylist;
	while(!list_empty(list)) {
		Node *entry = (Node*)list_remove(list_begin(list));
		visitor(entry);
	}
}

static void gen_param(Node *node) {
	List *list = &node->bodylist;
	while(!list_empty(list)) {
		Node *entry = (Node*)list_remove(list_back(list));
		gen_store(entry);
	}
}

static void gen_arg(Node *node) {
	List *list = &node->bodylist;
	while(!list_empty(list)) {
		Node *entry = (Node*)list_remove(list_back(list));
		visitor(entry);
	}
}

static void gen_method(Node *node) {
	method = emit_method(class, strndup(node->token->data, node->token->length));

	visitor(node->args);

	List *list = &node->bodylist;
	while(!list_empty(list)) {
		Node *entry = (Node*)list_remove(list_begin(list));
		visitor(entry);
	}
}

static void gen_if(Node *node) {
	int start = emit_op_get_counter(method);

	visitor(node->condition);
	emit_op_left(method, OP_LOAD_NUMBER, 0);
	Op *jmp = emit_op_left(method, OP_JMPIFT, 0);
	visitor(node->body);
	emit_op_left(method, OP_JMP, start);

	jmp->left = emit_op_get_counter(method);
}

static void gen_while(Node *node) {
	int start = emit_op_get_counter(method);

	visitor(node->condition);
	emit_op_left(method, OP_LOAD_NUMBER, 0);
	Op *jmp = emit_op_left(method, OP_JMPIFT, 0);
	visitor(node->body);
	emit_op_left(method, OP_JMP, start);

	jmp->left = emit_op_get_counter(method);
}

static void gen_block(Node *node) {
	List *list = &node->bodylist;
	while(!list_empty(list)) {
		Node *entry = (Node*)list_remove(list_begin(list));
		visitor(entry);
	}
}

static void gen_declaration(Node *node) {
	visitor(node->body);
	char *str = strndup(node->token->data, node->token->length);
	emit_op_left_string(method, OP_STORE_VAR, str);
}

static void gen_variable(Node *node) {
	char *str = strndup(node->token->data, node->token->length);
	emit_op_left_string(method, OP_LOAD_VAR, str);
}

static void gen_member(Node *node) {
	if(node->body) {
		visitor(node->body);
		char *str = strndup(node->token->data, node->token->length);
		emit_op_left_string(method, OP_LOAD_MEMBER, str);
	}
}

static void gen_new(Node *node) {
	char *str = strndup(node->token->data, node->token->length);
	emit_op_left_string(method, OP_LOAD_CONST, str);

	visitor(node->args);

	emit_op_left(method, OP_NEW, node->args->length);
}

static void gen_assign(Node *node) {
	visitor(node->right);
	gen_store(node->left);
}

static void gen_store(Node *node) {
	if(node->body) {
		visitor(node->body);
		char *str = strndup(node->token->data, node->token->length);
		emit_op_left_string(method, OP_STORE_MEMBER, str);
	} else {
		char *str = strndup(node->token->data, node->token->length);
		emit_op_left_string(method, OP_STORE_VAR, str);
	}

}

static void gen_binary(Node *node) {
	if(node->left) {
		visitor(node->left);
	}

	if(node->right) {
		visitor(node->right);
	}

	switch(node->type) {
		case ND_GT: {
			emit_op(method, OP_CMPGT);
		}
		break;
		case ND_LT: {
			emit_op(method, OP_CMPLT);
		}
		break;
		case ND_ADD: {
			emit_op(method, OP_ADD);
		}
		break;
		case ND_SUB: {
			emit_op(method, OP_SUB);
		}
		break;
		case ND_MUL: {
			emit_op(method, OP_MUL);
		}
		break;
		case ND_DIV: {
			emit_op(method, OP_DIV);
		}
		break;
		case ND_MOD: {
			emit_op(method, OP_MOD);
		}
		break;
	}
}

static void gen_number(Node *node) {
	char *str = strndup(node->token->data, node->token->length);
	emit_op_left(method, OP_LOAD_NUMBER, atof(str));
	free(str);
}

static void gen_string(Node *node) {
	char *str = strndup(node->token->data, node->token->length);
	emit_op_left_string(method, OP_LOAD_CONST, str);
}

static void gen_return(Node *node) {
	visitor(node->body);

	emit_op(method, OP_RET);
}

static void gen_call(Node *node) {
	visitor(node->body);
	visitor(node->args);

	emit_op_left(method, OP_CALL, node->args->length);
}

static void visitor(Node *node) {
	switch(node->type) {
		case ND_PROGRAM: {
			gen_program(node);
		}
		break;
		case ND_CLASS: {
			gen_class(node);
		}
		break;
		case ND_PARAM: {
			gen_param(node);
		}
		break;
		case ND_ARG: {
			gen_arg(node);
		}
		break;
		case ND_METHOD: {
			gen_method(node);
		}
		break;
		case ND_IF: {
			gen_if(node);
		}
		break;
		case ND_WHILE: {
			gen_while(node);
		}
		break;
		case ND_BLOCK: {
			gen_block(node);
		}
		break;
		case ND_DECL: {
			gen_declaration(node);
		}
		break;
		case ND_VARIABLE: {
			gen_variable(node);
		}
		break;
		case ND_MEMBER: {
			gen_member(node);
		}
		break;
		case ND_NEW: {
			gen_new(node);
		}
		break;
		case ND_ASSIGN: {
			gen_assign(node);
		}
		break;
		case ND_GT:
		case ND_LT:
		case ND_ADD:
		case ND_SUB:
		case ND_MUL:
		case ND_DIV:
		case ND_MOD: {
			gen_binary(node);
		}
		break;
		case ND_NUMBER: {
			gen_number(node);
		}
		break;
		case ND_STRING: {
			gen_string(node);
		}
		break;
		case ND_RETURN: {
			gen_return(node);
		}
		break;
		case ND_CALL: {
			gen_call(node);
		}
		break;
	}
}

void gen(Node *node, List *p) {
	list_clear(p);
	program = p;
	visitor(node);
	emit_print(p);
}