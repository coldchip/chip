#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "chip.h"
#include "gen.h"

typedef struct {
	TyMethod *method;
	int line;
} LabelEntry;

static LabelEntry labels[8192] = {};
int label_counter = 0;

static Op *codes[32768] = {};
int code_counter = 0;

static List constants;
static List *program;
static Class *class = NULL;
static Method *method = NULL;


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
	ins->has_left = false;
	list_insert(list_end(&method->op), ins);

	codes[code_counter++] = ins;

	return ins;
}

static Op *emit_op_left(Method *method, OpType op, uint64_t left) {
	Op *ins = malloc(sizeof(Op));
	ins->op = op;
	ins->has_left = true;
	ins->left = left;
	list_insert(list_end(&method->op), ins);

	codes[code_counter++] = ins;

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
	return code_counter + 1;
}

static void emit_file(List *constants, List *program) {
	Ty *c = type_get("Main");
	if(!c) {
		printf("entry point class Main not found\n");
		exit(1);
	}

	TyMethod *m = type_get_method(c, "main");
	if(!m) {
		printf("entry point method main not found\n");
		exit(1);
	}


	FILE *prg = fopen("~prg.out", "wb");

	for(int i = 0; i < label_counter; ++i) {
		if(labels[i].method == m) {
			uint32_t entry = labels[i].line;
			fwrite(&entry, sizeof(int), 1, prg);
			break;
		}
	}

	fwrite(&code_counter, sizeof(int), 1, prg);

	for(int i = 0; i < code_counter; i++) {
		Op *ins = codes[i];

		char op = ins->op;

		if(ins->has_left) {
			op = op | (1 << 7);
		}
				
		fwrite(&op, sizeof(char), 1, prg);
		if(ins->has_left) {
			fwrite(&ins->left, sizeof(uint64_t), 1, prg);
		}
	}

	fclose(prg);

	FILE *cst = fopen("~cst.out", "wb");

	int constants_size = list_size(constants);
	fwrite(&constants_size, sizeof(constants_size), 1, cst);

	for(ListNode *c = list_begin(constants); c != list_end(constants); c = list_next(c)) {
		Constant *constant = (Constant*)c;

		if(
			false &&
			constant->obfuscated && 
			!strcasecmp(constant->data, "this") == 0 && 
			!strcasecmp(constant->data, "count") == 0
		) {

			char obfuscated2[1024] = "";

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

	for(int i = 0; i < prg_size; ++i) {
		char ch = fgetc(prg);
		fputc(ch, fp);
	}

	fclose(prg);

	cst = fopen("~cst.out", "rb");
	fseek(cst, 0, SEEK_END);
	int cst_size = ftell(cst);
	fseek(cst, 0, SEEK_SET);

	fwrite(&cst_size, sizeof(cst_size), 1, fp);

	for(int i = 0; i < cst_size; ++i) {
		char ch = fgetc(cst);
		fputc(ch, fp);
	}

	fclose(cst);
	fclose(fp);

	unlink("~prg.out");
	unlink("~cst.out");
}

static void gen_program(Node *node) {
	while(!list_empty(&node->bodylist)) {
		Node *entry = (Node*)list_remove(list_begin(&node->bodylist));
		gen_visitor(entry);
	}
}

static void gen_class(Node *node) {
	class = emit_class(program, node->token->data);

	while(!list_empty(&node->bodylist)) {
		Node *entry = (Node*)list_remove(list_begin(&node->bodylist));
		gen_visitor(entry);
	}
}

static int gen_param(Node *node) {
	int i = 0;
	while(!list_empty(&node->bodylist)) {
		Node *entry = (Node*)list_remove(list_back(&node->bodylist));
		gen_store(entry);
		i++;
	}

	return i;
}

static int gen_arg(Node *node) {
	int i = 0;
	while(!list_empty(&node->bodylist)) {
		Node *entry = (Node*)list_remove(list_back(&node->bodylist));
		gen_visitor(entry);
		i++;
	}

	return i;
}

static void gen_method(Node *node) {
	method = emit_method(class, node->token->data);
	method->modifier = node->modifier;

	LabelEntry label = {
		.method = node->method,
		.line = emit_op_get_counter(method)
	};

	labels[label_counter++] = label;

	gen_visitor(node->args);

	while(!list_empty(&node->bodylist)) {
		Node *entry = (Node*)list_remove(list_begin(&node->bodylist));
		gen_visitor(entry);
	}

	emit_op_left(method, OP_PUSH, 0);
	emit_op(method, OP_RET);
}

static void gen_if(Node *node) {
	int start = emit_op_get_counter(method);

	gen_visitor(node->condition);
	emit_op_left(method, OP_PUSH, 0);
	Op *jmp = emit_op_left(method, OP_JE, 0);
	gen_visitor(node->body);

	Op *jmp2 = emit_op_left(method, OP_JMP, 0);

	jmp->left = emit_op_get_counter(method);
	if(node->alternate) {
		gen_visitor(node->alternate);
	}

	jmp2->left = emit_op_get_counter(method);
}

static void gen_while(Node *node) {
	int start = emit_op_get_counter(method);

	gen_visitor(node->condition);
	emit_op_left(method, OP_PUSH, 0);
	Op *jmp = emit_op_left(method, OP_JE, 0);
	gen_visitor(node->body);
	emit_op_left(method, OP_JMP, start);

	jmp->left = emit_op_get_counter(method);
}

static void gen_block(Node *node) {
	while(!list_empty(&node->bodylist)) {
		Node *entry = (Node*)list_remove(list_begin(&node->bodylist));
		gen_visitor(entry);
	}
}

static void gen_variable(Node *node) {
	emit_op_left(method, OP_LOAD, emit_constant(&constants, node->token->data, true));
}

static void gen_member(Node *node) {
	if(node->body) {
		gen_visitor(node->body);
		emit_op_left(method, OP_LOAD_MEMBER, emit_constant(&constants, node->token->data, true));
	}
}

static void gen_new(Node *node) {
	// emit_op_left(method, OP_LOAD_CONST, emit_constant(&constants, node->token->data, true));

	// gen_visitor(node->args);

	emit_op_left(method, OP_NEWO, emit_constant(&constants, node->token->data, true));
}

static void gen_new_array(Node *node) {
	gen_visitor(node->args);

	emit_op_left(method, OP_NEWARRAY, emit_constant(&constants, node->token->data, true));
}

static void gen_array_member(Node *node) {
	if(node->body) {
		gen_visitor(node->body);
		gen_visitor(node->index);
		emit_op(method, OP_LOAD_ARRAY);
	}
}

static void gen_expr(Node *node) {
	gen_visitor(node->body);

	// assignless
	emit_op(method, OP_POP);
}

static void gen_decl(Node *node) {
	if(node->body) {
		gen_visitor(node->body);
		emit_op_left(method, OP_STORE, emit_constant(&constants, node->token->data, true));
	}
}

static void gen_assign(Node *node) {
	gen_visitor(node->right);
	gen_store(node->left);
}

static void gen_store(Node *node) {
	if(node->body) {
		gen_visitor(node->body);
		if(node->index) {
			/* x[y] = z */
			gen_visitor(node->index);
			emit_op(method, OP_STORE_ARRAY);
		} else {
			/* x.y = z */
			emit_op_left(method, OP_STORE_MEMBER, emit_constant(&constants, node->token->data, true));
		}
	} else {
		/* x = y */
		emit_op_left(method, OP_STORE, emit_constant(&constants, node->token->data, true));
	}

}

static void gen_binary(Node *node) {
	if(node->left) {
		gen_visitor(node->left);
	}

	if(node->right) {
		gen_visitor(node->right);
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
		case ND_OR: {
			emit_op(method, OP_OR);
		}
		break;
	}
}

static void gen_char(Node *node) {
	int t = (int)node->token->data[0];

	emit_op_left(method, OP_PUSH, (float)node->token->data[0]);
}

static void gen_number(Node *node) {
	emit_op_left(method, OP_PUSH, atof(node->token->data));
}

static void gen_float(Node *node) {
	emit_op_left(method, OP_PUSH, atof(node->token->data));
}

static void gen_string(Node *node) {
	emit_op_left(method, OP_LOAD_CONST, emit_constant(&constants, node->token->data, false));
}

static void gen_return(Node *node) {
	gen_visitor(node->body);
	emit_op(method, OP_RET);
}

static void gen_call(Node *node) {
	int arg_count = gen_arg(node->args);
	gen_visitor(node->body);

	printf("neneneenenene %s() %p %li\n", node->token->data, node->method, list_size(&node->args->bodylist));

	for(int i = 0; i < label_counter; ++i) {
		if(labels[i].method == node->method) {
			uint32_t left = (arg_count << 24) | (labels[i].line);
			emit_op_left(method, OP_CALL, left);
			break;
		}
	}
}

static void gen_syscall(Node *node) {
	gen_visitor(node->args);
	emit_op_left(method, OP_SYSCALL, list_size(&node->args->bodylist));
}

static void gen_visitor(Node *node) {
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
		case ND_NEWARRAY: {
			gen_new_array(node);
		}
		break;
		case ND_ARRAYMEMBER: {
			gen_array_member(node);
		}
		break;
		case ND_EXPR: {
			gen_expr(node);
		}
		break;
		case ND_DECL: {
			gen_decl(node);
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
		case ND_MOD:
		case ND_OR: {
			gen_binary(node);
		}
		break;
		case ND_CHAR: {
			gen_char(node);
		}
		break;
		case ND_NUMBER: {
			gen_number(node);
		}
		break;
		case ND_FLOAT: {
			gen_float(node);
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
	gen_visitor(node);

	// print_code();
	emit_file(&constants, p);
}