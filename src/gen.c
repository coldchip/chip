#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "chip.h"

static char *rand_string(char *str, size_t size) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyz";
    if(size) {
        --size;
        for (size_t n = 0; n < size; n++) {
            int key = rand() % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
    return str;
}

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

static int emit_constant(List *list, char *data, bool obfuscated) {
	int i = 0;
	for(ListNode *c = list_begin(list); c != list_end(list); c = list_next(c)) {
		Constant *constant = (Constant*)c;
		if(strcmp(data, constant->data) == 0 && constant->obfuscated == obfuscated) {
			return i;
		}
		i++;
	}

	Constant *constant = malloc(sizeof(Constant));
	constant->data = data;
	constant->obfuscated = obfuscated;
	list_insert(list_end(list), constant);
	return list_size(list) - 1;
}

static int emit_op_get_counter(Method *program) {
	return list_size(&program->op) + 1;
}

static void emit_file(List *constants, List *program) {
	FILE *prg = fopen("~prg.out", "wb");

	int class_count = list_size(program);
	fwrite(&class_count, sizeof(int), 1, prg);

	for(ListNode *cn = list_begin(program); cn != list_end(program); cn = list_next(cn)) {
		Class *c = (Class*)cn;

		short method_count = list_size(&c->method);
		short class_name = emit_constant(constants, c->name, true);
		fwrite(&method_count, sizeof(short), 1, prg);
		fwrite(&class_name, sizeof(short), 1, prg);

		for(ListNode *mn = list_begin(&c->method); mn != list_end(&c->method); mn = list_next(mn)) {
			Method *m = (Method*)mn;
			
			short op_count = list_size(&m->op);
			short method_name = emit_constant(constants, m->name, true);
			fwrite(&op_count, sizeof(short), 1, prg);
			fwrite(&method_name, sizeof(short), 1, prg);

			for(ListNode *op = list_begin(&m->op); op != list_end(&m->op); op = list_next(op)) {
				Op *ins = (Op*)op;
				
				fwrite(&ins->op, sizeof(char), 1, prg);
				fwrite(&ins->left, sizeof(double), 1, prg);
			}
		}
	}

	fclose(prg);

	FILE *cst = fopen("~cst.out", "wb");

	int constants_size = list_size(constants);
	fwrite(&constants_size, sizeof(constants_size), 1, cst);

	for(ListNode *c = list_begin(constants); c != list_end(constants); c = list_next(c)) {
		Constant *constant = (Constant*)c;

		printf("%s %i\n", constant->data, constant-> obfuscated);

		if(
			constant->obfuscated && 
			!strcasecmp(constant->data, "main") == 0 && 
			!strcasecmp(constant->data, "this") == 0 && 
			!strcasecmp(constant->data, "constructor") == 0 &&
			!strcasecmp(constant->data, "string") == 0 &&
			!strcasecmp(constant->data, "number") == 0
		) {
			char obfuscated[512];
			rand_string(obfuscated, 8);

			char obfuscated2[1024];
			strcpy(obfuscated2, obfuscated);

			int constant_size = strlen(obfuscated2);
			fwrite(&constant_size, sizeof(constant_size), 1, cst);
			fwrite(obfuscated2, sizeof(char), constant_size, cst);
		} else {

			int constant_size = strlen(constant->data);
			fwrite(&constant_size, sizeof(constant_size), 1, cst);
			fwrite(constant->data, sizeof(char), constant_size, cst);
		}
	}

	fclose(cst);

	FILE *fp = fopen("a.out", "wb");

	char magic[8] = "CHIP";
	fwrite(magic, sizeof(char), 8, fp);

	prg = fopen("~prg.out", "rb");
	fseek(prg, 0, SEEK_END);
	int prg_size = ftell(prg);
	fseek(prg, 0, SEEK_SET);

	fwrite(&prg_size, sizeof(prg_size), 1, fp);

	char ch;
	while((ch = fgetc(prg)) != EOF) {
		fputc(ch, fp);
	}

	fclose(prg);



	cst = fopen("~cst.out", "rb");
	fseek(cst, 0, SEEK_END);
	int cst_size = ftell(cst);
	fseek(cst, 0, SEEK_SET);

	fwrite(&cst_size, sizeof(cst_size), 1, fp);

	while((ch = fgetc(cst)) != EOF) {
		fputc(ch, fp);
	}

	fclose(cst);

	fclose(fp);

	unlink("~prg.out");
	unlink("~cst.out");
}
static List constants;
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
	class = emit_class(program, node->token->data);

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
	method = emit_method(class, node->token->data);
	method->modifier = node->modifier;

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

	Op *jmp2 = emit_op_left(method, OP_JMP, 0);

	jmp->left = emit_op_get_counter(method);
	if(node->alternate) {
		visitor(node->alternate);
	}

	jmp2->left = emit_op_get_counter(method);
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

static void gen_variable(Node *node) {
	emit_op_left(method, OP_LOAD_VAR, emit_constant(&constants, node->token->data, true));
}

static void gen_member(Node *node) {
	if(node->body) {
		visitor(node->body);
		emit_op_left(method, OP_LOAD_MEMBER, emit_constant(&constants, node->token->data, true));
	}
}

static void gen_new(Node *node) {
	emit_op_left(method, OP_LOAD_CONST, emit_constant(&constants, node->token->data, true));

	visitor(node->args);

	emit_op_left(method, OP_NEW, node->args->length);
}

static void gen_expr(Node *node) {
	visitor(node->body);

	// assignless
	emit_op(method, OP_POP);
}

static void gen_assign(Node *node) {
	visitor(node->right);
	gen_store(node->left);
}

static void gen_store(Node *node) {
	if(node->body) {
		/* x.y = z */
		visitor(node->body);
		emit_op_left(method, OP_STORE_MEMBER, emit_constant(&constants, node->token->data, true));
	} else {
		/* x = y */
		emit_op_left(method, OP_STORE_VAR, emit_constant(&constants, node->token->data, true));
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
		case ND_EQ: {
			emit_op(method, OP_CMPEQ);
		}
		break;
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
	emit_op_left(method, OP_LOAD_NUMBER, atof(node->token->data));
}

static void gen_string(Node *node) {
	emit_op_left(method, OP_LOAD_CONST, emit_constant(&constants, node->token->data, false));
}

static void gen_return(Node *node) {
	visitor(node->body);

	emit_op(method, OP_RET);
}

static void gen_call(Node *node) {
	visitor(node->args);
	visitor(node->body);

	emit_op_left(method, OP_CALL, node->args->length);
}

static void gen_syscall(Node *node) {
	visitor(node->args);
	emit_op_left(method, OP_SYSCALL, node->args->length);
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
		case ND_EXPR: {
			gen_expr(node);
		}
		break;
		case ND_ASSIGN: {
			gen_assign(node);
		}
		break;
		case ND_EQ:
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
		case ND_SYSCALL: {
			gen_syscall(node);
		}
		break;
	}
}

void gen(Node *node, List *p) {
	list_clear(&constants);
	list_clear(p);
	program = p;
	visitor(node);
	emit_file(&constants, p);
}