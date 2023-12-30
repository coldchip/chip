#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "chip.h"
#include "parse.h"
#include "varscope.h"
#include "semantic.h"
#include "type.h"

/*
	Chip semantic analyzer
*/

Ty *return_ty = NULL;

/*
	process class & method types without statements 
*/

void semantic_peek_class(Node *node) {
	for(ListNode *c = list_begin(&node->bodylist); c != list_end(&node->bodylist); c = list_next(c)) {
		Node *class = (Node*)c;
		if(class->type != ND_CLASS) continue;

		if(type_get(class->token->data)) {
			printf("error, redefinition of class %s\n", class->token->data);
			exit(1);
		}
		type_insert(class->token->data, 8);
	}


	for(ListNode *c = list_begin(&node->bodylist); c != list_end(&node->bodylist); c = list_next(c)) {
		Node *class = (Node*)c;
		if(class->type != ND_CLASS) continue;

		Ty *class_ty = type_get(class->token->data);

		for(ListNode *m = list_begin(&class->bodylist); m != list_end(&class->bodylist); m = list_next(m)) {
			Node *method = (Node*)m;

			switch(method->type) {
				case ND_CLASS_DECL: {
					Node *type = method->data_type;
					Ty *ty = type_get(type->token->data);
					if(!ty) {
						printf("unknown declaration type: %s\n", type->token->data);
						exit(1);
					}

					if(type_get_variable(class_ty, method->token->data)) {
						printf("error, redefinition of class variable %s\n", method->token->data);
						exit(1);
					}

					insert_variable(class_ty, method->token->data, ty);
				}
				break;
				case ND_METHOD: {
					Node *type = method->data_type;
					Ty *ty = type_get(type->token->data);
					if(!ty) {
						printf("unknown return type: %s\n", type->token->data);
						exit(1);
					}

					char *signature = semantic_param_signature(method->args);

					if(type_get_method(class_ty, method->token->data, signature)) {
						printf("error, redefinition of method %s(%s)\n", method->token->data, signature);
						exit(1);
					}

					method->method = insert_method(class_ty, method->token->data, signature, ty);
				}
				break;
			}
		}
	}
	printf("\n\n");
}

void semantic_program(Node *node) {
	for(ListNode *n = list_begin(&node->bodylist); n != list_end(&node->bodylist); n = list_next(n)) {
		Node *entry = (Node*)n;
		semantic_class(entry);
	}
}

void semantic_class(Node *node) {
	printf(" - class %s\n", node->token->data);

	Ty *ty = type_get(node->token->data);

	for(ListNode *n = list_begin(&node->bodylist); n != list_end(&node->bodylist); n = list_next(n)) {
		Node *entry = (Node*)n;
		switch(entry->type) {
			case ND_CLASS_DECL: {
				// semantic_class_decl(entry);
			}
			break;
			case ND_METHOD: {
				varscope_push();
				varscope_add("this", ty);
				semantic_method(entry);
				varscope_pop();
			}
			break;
		}
	}
}

void semantic_class_decl(Node *node) {
	Node *type = node->data_type;
	Ty *ty = type_get(type->token->data);
	if(!ty) {
		printf("unknown type %s in declaration\n", type->token->data);
		exit(1);
	}

	printf("\t - %s %s;\n", type->token->data, node->token->data);
}

char *semantic_param_signature(Node *node) {
	char result[8192];
	strcpy(result, "");

	if(list_size(&node->bodylist) == 0) {
		strcat(result, "void;");
	}

	for(ListNode *p = list_begin(&node->bodylist); p != list_end(&node->bodylist); p = list_next(p)) {
		Node *param = (Node*)p;

		Node *type = param->data_type;
		Ty *ty = type_get(type->token->data);
		if(!ty) {
			printf("unknown type %s in parameter\n", type->token->data);
			exit(1);
		}

		strcat(result, ty->name);
		strcat(result, ";");
	}

	return strdup(result);
}

char *semantic_arg_signature(Node *node) {
	char result[8192];
	strcpy(result, "");

	if(list_size(&node->bodylist) == 0) {
		strcat(result, "void;");
	}

	for(ListNode *a = list_begin(&node->bodylist); a != list_end(&node->bodylist); a = list_next(a)) {
		Node *arg = (Node*)a;
		Ty *ty = semantic_walk_expr(arg);

		strcat(result, ty->name);
		strcat(result, ";");
	}

	return strdup(result);
}

void semantic_param(Node *node) {
	for(ListNode *p = list_begin(&node->bodylist); p != list_end(&node->bodylist); p = list_next(p)) {
		Node *param = (Node*)p;

		Node *type = param->data_type;
		Ty *ty = type_get(type->token->data);
		if(!ty) {
			printf("unknown type %s in parameter\n", type->token->data);
			exit(1);
		}

		if(varscope_get(param->token->data)) {
			printf("error, redefinition of parameter %s\n", param->token->data);
			exit(1);
		}

		Var *var = varscope_add(param->token->data, ty);
		param->offset = var->offset;
	}
}

void semantic_arg(Node *node) {
	for(ListNode *a = list_begin(&node->bodylist); a != list_end(&node->bodylist); a = list_next(a)) {
		Node *arg = (Node*)a;
		semantic_walk_expr(arg);
	}
}

void semantic_method(Node *node) {
	Node *type = node->data_type;
	return_ty = type_get(type->token->data);

	printf("\t - method %s(%s) : %s\n", node->token->data, node->method->signature, return_ty->name);

	if(node->args) {
		semantic_param(node->args);
	}

	for(ListNode *s = list_begin(&node->bodylist); s != list_end(&node->bodylist); s = list_next(s)) {
		Node *stmt = (Node*)s;
		semantic_stmt(stmt);

	}
}

void semantic_stmt(Node *node) {
	switch(node->type) {
		case ND_IF: {
			semantic_if(node);
		}
		break;
		case ND_WHILE: {
			semantic_while(node);
		}
		break;
		case ND_BLOCK: {
			for(ListNode *s = list_begin(&node->bodylist); s != list_end(&node->bodylist); s = list_next(s)) {
				Node *stmt = (Node*)s;
				semantic_stmt(stmt);
			}
		}
		break;
		case ND_RETURN: {
			semantic_return(node);
		}
		break;
		case ND_DECL: {
			semantic_decl(node);
		}
		break;
		case ND_EXPR: {
			semantic_expr(node);
		}
		break;
	}
}

void semantic_if(Node *node) {
	semantic_walk_expr(node->condition);
	semantic_stmt(node->body);
	if(node->alternate) {
		semantic_stmt(node->alternate);
	}
}

void semantic_while(Node *node) {
	semantic_walk_expr(node->condition);
	semantic_stmt(node->body);
}

void semantic_return(Node *node) {
	Ty *ty = type_get("void");
	if(node->body) {
		ty = semantic_walk_expr(node->body);
	}

	if(ty != return_ty) {
		printf("warning: returning type of: %s, expected: %s\n", ty->name, return_ty->name);
	}
}

void semantic_decl(Node *node) {
	Node *type = node->data_type;
	Ty *left = type_get(type->token->data);
	if(!left) {
		printf("unknown type %s in declaration\n", type->token->data);
		exit(1);
	}

	if(varscope_get(node->token->data)) {
		printf("error, redefinition of variable %s\n", node->token->data);
		exit(1);
	}

	Var *var = varscope_add(node->token->data, left);
	node->offset = var->offset;

	if(node->body) {
		semantic_walk_expr(node->body);
	}
}

void semantic_expr(Node *node) {
	semantic_walk_expr(node->body);
}

Ty *semantic_walk_expr(Node *node) {
	switch(node->type) {
		case ND_ASSIGN: {
			Ty *left  = semantic_walk_expr(node->left);
			Ty *right = semantic_walk_expr(node->right);

			if(left != right && !type_compatible(right, left)) {
				printf("error: incompatible types: cannot assign %s to %s\n", right->name, left->name);
				exit(1);
			}

			return left;
		}
		break;
		case ND_EQ:
		case ND_GT:
		case ND_LT:
		case ND_ADD:
		case ND_SUB:
		case ND_MUL:
		case ND_DIV:
		case ND_MOD:
		case ND_OR: {
			Ty *left  = semantic_walk_expr(node->left);
			Ty *right = semantic_walk_expr(node->right);
			Ty *common = type_get_common(left, right);
			if(!common || !type_compatible(left, common) || !type_compatible(right, common)) {
				printf("error: incompatible types: %s cannot perform arithmetric operation to %s\n", right->name, left->name);
				exit(1);
			}

			if(left != common) {
				Node *cast_left = new_node(ND_CAST, NULL);
				cast_left->body = node->left;
				node->left = cast_left;
			}

			if(right != common) {
				Node *cast_right = new_node(ND_CAST, NULL);
				cast_right->body = node->right;
				node->right = cast_right;
			}

			return common;
		}
		break;
		case ND_NEG:
		case ND_NOT: {
			return semantic_walk_expr(node->body);
		}
		break;
		case ND_CAST: {
			return semantic_walk_expr(node->body);
		}
		break;
		case ND_MEMBER: {
			Ty *parent = semantic_walk_expr(node->body);

			TyVariable *variable = type_get_variable(parent, node->token->data);
			if(!variable) {
				printf("unknown member %s\n", node->token->data);
				exit(1);
			}

			node->offset = variable->offset;

			return variable->type;
		}
		break;
		case ND_CALL: {
			Ty *parent = semantic_walk_expr(node->body);

			char *signature = semantic_arg_signature(node->args);

			TyMethod *method = type_get_method(parent, node->token->data, signature);
			if(!method) {
				printf("call to unknown member %s(%s)\n", node->token->data, signature);
				exit(1);
			}

			semantic_arg(node->args);

			node->method = method;

			return method->type;
		}
		break;
		case ND_VARIABLE: {
			Var *var = varscope_get(node->token->data);
			Ty  *ty  = type_get(node->token->data);

			if(!var && !ty) {
				printf("undefined variable %s\n", node->token->data);
				exit(1);
			}

			node->offset = var ? var->offset : 0;

			return var ? var->type : ty;
		}
		break;
		case ND_NUMBER: {
			return type_get("int");
		}
		break;
		case ND_FLOAT: {
			return type_get("float");
		}
		break;
		case ND_CHAR: {
			return type_get("char");
		}
		break;
		case ND_STRING: {
			return type_get("char");
		}
		break;
		case ND_SYSCALL: {
			Node *type = node->data_type;
			Ty *ty = type_get(type->token->data);
			if(!ty) {
				printf("unknown type %s\n", type->token->data);
				exit(1);
			}

			semantic_arg(node->args);

			return ty;
		}
		break;
		case ND_NEW: {
			Node *type = node->data_type;
			Ty *ty = type_get(type->token->data);
			if(!ty) {
				printf("unknown type %s\n", type->token->data);
				exit(1);
			}

			semantic_arg(node->args);

			char *signature  = semantic_arg_signature(node->args);
			TyMethod *method = type_get_method(ty, "constructor", signature);
			if(!method) {
				printf("call to unknown constructor %s of type %s\n", signature, type->token->data);
				exit(1);
			}

			node->size   = type_size(ty);
			node->method = method;

			return ty;
		}
		break;
		case ND_NEWARRAY: {
			Node *type = node->data_type;
			Ty *ty = type_get(type->token->data);
			if(!ty) {
				printf("unknown type %s\n", type->token->data);
				exit(1);
			}

			semantic_walk_expr(node->args);

			return ty;
		}
		break;
		case ND_ARRAYMEMBER: {
			semantic_walk_expr(node->index);
			return semantic_walk_expr(node->body);
		}
		break;
		default: {
			printf("unknown node %i\n", node->type);
			exit(1);
		}
		break;
	}
}

void semantic(Node *node) {
	type_clear();
	varscope_clear();

	semantic_peek_class(node);
	semantic_program(node);
}