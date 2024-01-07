#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "chip.h"
#include "optimize.h"
#include "codegen.h"

static LabelEntry labels[8192] = {};
int label_counter = 0;

static Op *codes[32768] = {};
int code_counter = 0;

static char raw[32768] = {};
int raw_counter = 0;

static List constants;

static int rand_string() {
	static int result = 0;
    return result++;
}

int testtest(int a) {
	int cl = 0;
	for(int i = 0; i < code_counter; i++) {
		Op *current = codes[i];

		if(i == a) {
			return cl;
		}

		uint8_t op    = current->op;
		int64_t left  = current->left;
		uint8_t width = closest_container_size(current->left);

		if(op_jmp[op]) {
			width = 2;
		}

		cl++;
		if(op_size[op]) {
			cl += (1 << width);
		}
	}
}

void emit_label_to_address() {
	for(int i = 0; i < code_counter; i++) {
		Op *ins = codes[i];

		if(ins->left_label) {
			LabelEntry label = emit_get_label(ins->left_label);
			ins->left = testtest(label.line);
		}
	}

	for(int i = 0; i < code_counter; ++i) {
		Op *current = codes[i];

		uint8_t op    = current->op;
		int64_t left  = current->left;
		uint8_t width = closest_container_size(left);

		if(op_jmp[op]) {
			width = 2;
		}

		uint8_t encoded_op   = (op << 2) | (width & 0x03);
		int64_t encoded_left = (left);

		raw[raw_counter] = encoded_op;
		raw_counter++;

		if(op_size[op]) {
			for(int i = 0; i < (1 << width); ++i) {
				uint8_t bit = ((uint8_t*)&encoded_left)[i] & 0xFF;
				
				raw[raw_counter] = bit;
				raw_counter++;
			}
		}
	}

	printf("code size %i\n", raw_counter);
}

LabelEntry emit_get_label(const char *name) {
	for(int i = 0; i < label_counter; ++i) {
		if(strcmp(labels[i].name, name) == 0) {
			return labels[i];
		}
	}
}

LabelEntry emit_label(const char *name) {
	LabelEntry label = {
		.line = emit_op_get_counter(),
	};
	strcpy(label.name, name);

	labels[label_counter++] = label;

	return label;
}

static Op *emit_op(OpType op) {
	return emit_op_left(op, 0);
}

static Op *emit_op_left(OpType op, uint64_t left) {
	Op *ins = malloc(sizeof(Op));
	ins->op = op;
	ins->left = left;
	ins->left_label = NULL;

	codes[code_counter++] = ins;

	return ins;
}

static Op *emit_op_left_label(OpType op, const char *left) {
	Op *ins = malloc(sizeof(Op));
	ins->op = op;
	ins->left_label = strdup(left);

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

static int emit_op_get_counter() {
	return code_counter;
}

static void emit_file(List *constants) {
	Ty *c = type_get("Main");
	if(!c) {
		printf("entry point class Main not found\n");
		exit(1);
	}

	TyMethod *m = type_get_method(c, "main", "void;");
	if(!m) {
		printf("entry point method main not found\n");
		exit(1);
	}

	FILE *prg = fopen("a.out", "wb");
	if(!prg) {
		printf("unable to open to ~prg.out\n");
		exit(1);
	}

	char magic[8] = "CHIP";
	fwrite(magic, sizeof(char), 8, prg);
	fwrite(&raw_counter, sizeof(raw_counter), 1, prg);

	char entry_label[256];
	sprintf(entry_label, "SUB_%p_%s", m, m->name);
	LabelEntry entry = emit_get_label(entry_label);

	uint32_t e = testtest(entry.line);

	fwrite(&e, sizeof(e), 1, prg);

	int pos = 0;

	for(int i = 0; i < raw_counter; i++) {
		fwrite(&raw[i], sizeof(char), 1, prg);
	}

	int constants_size = list_size(constants);
	fwrite(&constants_size, sizeof(constants_size), 1, prg);

	for(ListNode *c = list_begin(constants); c != list_end(constants); c = list_next(c)) {
		Constant *constant = (Constant*)c;
	
		int constant_size = strlen(constant->data);

		fwrite(&constant_size, sizeof(constant_size), 1, prg);
		fwrite(constant->data, sizeof(char), constant_size, prg);
		
	}

	fclose(prg);
}

void emit_asm() {
	for(int pc = 0; pc < code_counter; pc++) {
		Op *ins = codes[pc];

		for(int i = 0; i < label_counter; ++i) {
			if((labels[i].line - 1) == pc) {
				printf("%i:%s:\n", testtest(labels[i].line), labels[i].name);
				break;
			}
		}

		if(ins->left_label) {
			printf("\t%s\t%s@0x%02lx\n", op_display[ins->op], ins->left_label, ins->left);
		} else {
			if(op_size[ins->op]) {
				printf("\t%s\t%li\n", op_display[ins->op], ins->left);
			} else {
				printf("\t%s\n", op_display[ins->op]);
			}
		}
	}
}

uint8_t closest_container_size(int64_t number) {
	if(number == (int8_t)(number & 0xFF)) {
		return 0;
	} else if(number == (int16_t)(number & 0xFFFF)) {
		return 1;
	} else if(number == (int32_t)(number & 0xFFFFFFFF)) {
		return 2;
	} else if(number == (int64_t)(number & 0xFFFFFFFFFFFFFFFF)) {
		return 3;
	}
}

static void gen_program(Node *node) {
	while(!list_empty(&node->bodylist)) {
		Node *entry = (Node*)list_remove(list_begin(&node->bodylist));
		gen_visitor(entry);
	}
}

static void gen_import(Node *node) {
	gen_visitor(node->body);
}

static void gen_class(Node *node) {
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
	char label[256];
	sprintf(label, "SUB_%p_%s", node->method, node->method->name);

	emit_label(label);

	gen_visitor(node->args);

	/* this */
	emit_op_left(OP_STORE, 0);

	while(!list_empty(&node->bodylist)) {
		Node *entry = (Node*)list_remove(list_begin(&node->bodylist));
		gen_visitor(entry);
	}

	emit_op_left(OP_PUSH, 0);
	emit_op(OP_RET);
}

static void gen_if(Node *node) {
	char condition_label[256];
	sprintf(condition_label, "IB_%i", rand_string());

	char alternate_label[256];
	sprintf(alternate_label, "IA_%i", rand_string());

	char exit_label[256];
	sprintf(exit_label, "IE_%i", rand_string());

	// check
	gen_visitor(node->condition);
	emit_op_left(OP_PUSH, 1);
	emit_op_left_label(OP_JE, condition_label);
	emit_op_left_label(OP_JMP, alternate_label);

	// body
	emit_label(condition_label);
	gen_visitor(node->body);
	emit_op_left_label(OP_JMP, exit_label);

	// else
	emit_label(alternate_label);
	if(node->alternate) {
		gen_visitor(node->alternate);
	}
	emit_op_left_label(OP_JMP, exit_label);

	// exit
	emit_label(exit_label);
}

static void gen_while(Node *node) {
	char body_label[256];
	sprintf(body_label, "WB_%i", rand_string());

	char exit_label[256];
	sprintf(exit_label, "WE_%i", rand_string());

	// check
	emit_label(body_label);
	gen_visitor(node->condition);
	emit_op_left(OP_PUSH, 0);
	emit_op_left_label(OP_JE, exit_label);

	// body
	gen_visitor(node->body);
	emit_op_left_label(OP_JMP, body_label);

	// exit
	emit_label(exit_label);
}

static void gen_block(Node *node) {
	while(!list_empty(&node->bodylist)) {
		Node *entry = (Node*)list_remove(list_begin(&node->bodylist));
		gen_visitor(entry);
	}
}

static void gen_variable(Node *node) {
	emit_op_left(OP_LOAD, node->offset);
}

static void gen_member(Node *node) {
	if(node->body) {
		gen_visitor(node->body);
		emit_op_left(OP_LOAD_FIELD, node->offset);
	}
}

static void gen_new(Node *node) {
	emit_op_left(OP_NEWO, node->size);

	if(node->method) {
		emit_op(OP_DUP);
	
		int arg_count = gen_arg(node->args);

		char label[256];
		sprintf(label, "SUB_%p_%s", node->method, node->method->name);

		emit_op_left(OP_PUSH, arg_count);
		emit_op_left_label(OP_CALL, label);

		// discard constructor return
		emit_op(OP_POP);
	}
}

static void gen_new_array(Node *node) {
	gen_visitor(node->args);

	emit_op(OP_NEW_ARRAY);
}

static void gen_array_member(Node *node) {
	if(node->body) {
		gen_visitor(node->body);
		gen_visitor(node->index);
		emit_op(OP_LOAD_ARRAY);
	}
}

static void gen_expr(Node *node) {
	if(node->body) {
		gen_visitor(node->body);
		emit_op(OP_POP);
	}
}

static void gen_decl(Node *node) {
	if(node->body) {
		gen_visitor(node->body);
		emit_op(OP_POP);
	}
}

static void gen_assign(Node *node) {
	gen_visitor(node->right);
	emit_op(OP_DUP);
	gen_store(node->left);
}

static void gen_store(Node *node) {
	if(node->body) {
		gen_visitor(node->body);
		if(node->index) {
			/* x[y] = z */
			gen_visitor(node->index);
			emit_op(OP_STORE_ARRAY);
		} else {
			/* x.y = z */
			emit_op_left(OP_STORE_FIELD, node->offset);
		}
	} else {
		/* x = y */
		emit_op_left(OP_STORE, node->offset);
	}

}

static void gen_binary(Node *node) {
	char true_label[256];
	sprintf(true_label, "LT_%i", rand_string());
	char false_label[256];
	sprintf(false_label, "LF_%i", rand_string());
	char exit_label[256];
	sprintf(exit_label, "LE_%i", rand_string());

	if(node->left) {
		gen_visitor(node->left);
	}

	switch(node->type) {
		case ND_OR: {
			emit_op_left(OP_PUSH, 1);
			emit_op_left_label(OP_JE, true_label);
		}
		break;
		case ND_AND: {
			emit_op_left(OP_PUSH, 0);
			emit_op_left_label(OP_JE, false_label);
		}
		break;
	}

	if(node->right) {
		gen_visitor(node->right);
	}

	switch(node->type) {
		case ND_EQ: {
			emit_op(OP_CMPEQ);
		}
		break;
		case ND_GT: {
			emit_op(OP_CMPGT);
		}
		break;
		case ND_LT: {
			emit_op(OP_CMPLT);
		}
		break;
		case ND_ADD: {
			if(node->ty == type_get("float")) {
				emit_op(OP_FADD);
			} else {
				emit_op(OP_ADD);
			}
		}
		break;
		case ND_SUB: {
			if(node->ty == type_get("float")) {
				emit_op(OP_FSUB);
			} else {
				emit_op(OP_SUB);
			}
		}
		break;
		case ND_MUL: {
			if(node->ty == type_get("float")) {
				emit_op(OP_FMUL);
			} else {
				emit_op(OP_MUL);
			}
		}
		break;
		case ND_DIV: {
			if(node->ty == type_get("float")) {
				emit_op(OP_FDIV);
			} else {
				emit_op(OP_DIV);
			}
		}
		break;
		case ND_MOD: {
			if(node->ty == type_get("float")) {
				emit_op(OP_FMOD);
			} else {
				emit_op(OP_MOD);
			}
		}
		break;
		case ND_OR: {
			emit_op_left(OP_PUSH, 1);
			emit_op_left_label(OP_JE, true_label);
			emit_op_left_label(OP_JMP, false_label);

			// true
			emit_label(true_label);
			emit_op_left(OP_PUSH, 1);
			emit_op_left_label(OP_JMP, exit_label);

			// false
			emit_label(false_label);
			emit_op_left(OP_PUSH, 0);
			emit_op_left_label(OP_JMP, exit_label);

			// exit
			emit_label(exit_label);
		}
		break;
		case ND_AND: {
			emit_op_left(OP_PUSH, 0);
			emit_op_left_label(OP_JE, false_label);
			emit_op_left_label(OP_JMP, true_label);

			// true
			emit_label(true_label);
			emit_op_left(OP_PUSH, 1);
			emit_op_left_label(OP_JMP, exit_label);

			// false
			emit_label(false_label);
			emit_op_left(OP_PUSH, 0);
			emit_op_left_label(OP_JMP, exit_label);

			// exit
			emit_label(exit_label);
		}
		break;
	}
}

static void gen_neg(Node *node) {
	gen_visitor(node->body);
	if(node->ty == type_get("float")) {
		emit_op(OP_FNEG);
	} else {
		emit_op(OP_NEG);
	}
}

static void gen_not(Node *node) {
	gen_visitor(node->body);
	emit_op_left(OP_PUSH, 0);
	emit_op(OP_CMPEQ);
}

static void gen_cast(Node *node) {
	gen_visitor(node->body);

	if(node->ty == type_get("float")) {
		emit_op(OP_I2F);
	}
}

static void gen_char(Node *node) {
	emit_op_left(OP_PUSH, (float)node->token->data[0]);
}

static void gen_number(Node *node) {
	emit_op_left(OP_PUSH, atol(node->token->data));
}

static void gen_float(Node *node) {
	double  value   = atof(node->token->data);
	int64_t value_i = *(int64_t*)&value;
	emit_op_left(OP_PUSH, value_i);
}

static void gen_string(Node *node) {
	emit_op_left(OP_LOAD_CONST, emit_constant(&constants, node->token->data, false));

	// emit_op_left(OP_PUSH, strlen(node->token->data));
	// emit_op_left(OP_NEW_ARRAY, 0);
	// emit_op(OP_DUP);

	// emit_op_left(OP_STORE, 128);

	// for(int i = 0; i < strlen(node->token->data); i++) {
	// 	emit_op_left(OP_PUSH, (char)node->token->data[i] - 20);
	// 	emit_op_left(OP_PUSH, 20);
	// 	emit_op(OP_ADD);

	// 	emit_op_left(OP_LOAD, 128);
	// 	emit_op_left(OP_PUSH, i);
	// 	emit_op(OP_STORE_ARRAY);
	// }

}

static void gen_return(Node *node) {
	if(node->body) {
		gen_visitor(node->body);
	} else {
		emit_op_left(OP_PUSH, 0);
	}
	emit_op(OP_RET);
}

static void gen_call(Node *node) {
	gen_visitor(node->body);

	int arg_count = gen_arg(node->args);

	char label[256];
	sprintf(label, "SUB_%p_%s", node->method, node->method->name);

	emit_op_left(OP_PUSH, arg_count);
	emit_op_left_label(OP_CALL, label);
}

static void gen_syscall(Node *node) {
	gen_visitor(node->args);
	emit_op(OP_SYSCALL);
}

static void gen_visitor(Node *node) {
	switch(node->type) {
		case ND_PROGRAM: {
			gen_program(node);
		}
		break;
		case ND_IMPORT: {
			gen_import(node);
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
		case ND_OR:
		case ND_AND: {
			gen_binary(node);
		}
		break;
		case ND_NEG: {
			gen_neg(node);
		}
		break;
		case ND_NOT: {
			gen_not(node);
		}
		break;
		case ND_CAST: {
			gen_cast(node);
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

void gen(Node *node) {
	list_clear(&constants);
	gen_visitor(node);

	optimize_peephole(codes, code_counter);

	emit_label_to_address();
	emit_asm();

	emit_file(&constants);
}