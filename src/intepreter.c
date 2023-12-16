/*
	The Chip Language Intepreter
	@Copyright Ryan Loh 2023
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include "chip.h"
#include "list.h"
#include "intepreter.h"

Object *cache[8192] = {}; 
Object *globals[8192] = {}; // generate instances of all classes to allow static invoking
static char *constants[8192] = {};

static Op *codes[32768] = {};
int code_size = 0;

int load_file(const char *name) {
	FILE *fp = fopen(name, "rb");
	if(!fp) {
		printf("unable to load file %s\n", name);
		exit(1);
	}

	char magic[8];
	fread(magic, sizeof(char), 8, fp);

	int pgm_size = 0;
	fread(&pgm_size, sizeof(pgm_size), 1, fp);

	fseek(fp, pgm_size + 4, SEEK_CUR);

	int constants_count = 0;
	fread(&constants_count, sizeof(constants_count), 1, fp);

	for(int z = 0; z < constants_count; z++) {
		int constant_length = 0;
		fread(&constant_length, sizeof(constant_length), 1, fp);

		char constant_data[constant_length + 1];
		memset(constant_data, 0, sizeof(constant_data));

		fread(&constant_data, sizeof(char), constant_length, fp);

		constant_data[sizeof(constant_data) - 1] = '\0';

		SET_CONST(z, strdup(constant_data));
	}

	fseek(fp, sizeof(magic) + 4, SEEK_SET);

	int entry = 0;
	fread(&entry, sizeof(entry), 1, fp);

	int code_count = 0;
	fread(&code_count, sizeof(code_count), 1, fp);

	for(int i = 0; i < code_count; i++) {
		char     op = 0;
		int64_t op_left = 0;
		fread(&op, sizeof(op), 1, fp);
		if((op >> 7) & 0x01) {
			fread(&op_left, sizeof(op_left), 1, fp);
		}

		Op *ins = malloc(sizeof(Op));
		ins->op = op & 0x7F;
		ins->left = op_left;

		codes[code_size++] = ins;
	}

	fclose(fp);

	return entry;
}

void emit_print() {
	for(int pc = 0; pc < code_size; pc++) {
		Op *ins = codes[pc];
		switch(ins->op) {
			case OP_LOAD: {
				printf("\t%i\tload\t%s\n", pc + 1, GET_CONST(ins->left));
			}
			break;
			case OP_STORE: {
				printf("\t%i\tstore\t%s\n", pc + 1, GET_CONST(ins->left));
			}
			break;
			case OP_POP: {
				printf("\t%i\tpop\t\n", pc + 1);
			}
			break;
			case OP_CMPEQ: {
				printf("\t%i\tcmpeq\n", pc + 1);
			}
			break;
			case OP_CMPGT: {
				printf("\t%i\tcmpgt\n", pc + 1);
			}
			break;
			case OP_CMPLT: {
				printf("\t%i\tcmplt\n", pc + 1);
			}
			break;
			case OP_ADD: {
				printf("\t%i\tadd\n", pc + 1);
			}
			break;
			case OP_SUB: {
				printf("\t%i\tsub\n", pc + 1);
			}
			break;
			case OP_MUL: {
				printf("\t%i\tmul\n", pc + 1);
			}
			break;
			case OP_DIV: {
				printf("\t%i\tdiv\n", pc + 1);
			}
			break;
			case OP_MOD: {
				printf("\t%i\tmod\n", pc + 1);
			}
			break;
			case OP_OR: {
				printf("\t%i\tor\n", pc + 1);
			}
			break;
			case OP_DUP: {
				printf("\t%i\tdup\n", pc + 1);
			}
			break;
			case OP_PUSH: {
				printf("\t%i\tpush\t%li\n", pc + 1, ins->left);
			}
			break;
			case OP_LOAD_CONST: {
				printf("\t%i\tloadconst\t%i\t//%s\n", pc + 1, (int)ins->left, GET_CONST(ins->left));
			}
			break;
			case OP_LOAD_FIELD: {
				printf("\t%i\tloadmember\t%s\n", pc + 1, GET_CONST(ins->left));
			}
			break;
			case OP_STORE_FIELD: {
				printf("\t%i\tstoremember\t%s\n", pc + 1, GET_CONST(ins->left));
			}
			break;
			case OP_CALL: {
				uint32_t args = (((uint32_t)ins->left) >> 24) & 0xFF;
				uint32_t jmp = ((uint32_t)ins->left) & 0x00FFFFFF;
				printf("\t%i\tcall\t%i, %i\n", pc + 1, jmp, args);
			}
			break;
			case OP_SYSCALL: {
				printf("\t%i\tsyscall\t\tARGLEN: %i\n", pc + 1, (int)ins->left);
			}
			break;
			case OP_NEWO: {
				printf("\t%i\tnewo\t%s\n", pc + 1, GET_CONST(ins->left));
			}
			break;
			case OP_NEWARRAY: {
				printf("\t%i\tnew_array\t%s\n", pc + 1, GET_CONST(ins->left));
			}
			break;
			case OP_LOAD_ARRAY: {
				printf("\t%i\tload_array\n", pc + 1);
			}
			break;
			case OP_STORE_ARRAY: {
				printf("\t%i\tstore_array\n", pc + 1);
			}
			break;
			case OP_JE: {
				printf("\t%i\tje\t%i\n", pc + 1, (int)ins->left);
			}
			break;
			case OP_JMP: {
				printf("\t%i\tjmp\t%i\n", pc + 1, (int)ins->left);
			}
			break;
			case OP_RET: {
				printf("\t%i\tret\t\n", pc + 1);
			}
			break;
		}
	}
}

void store_var(double *vars, int index, Object *object) {
	*(Object **)&vars[index] = object;
}

void store_var_double(int64_t *vars, int index, int64_t data) {
	vars[index] = data;
}

double load_var(double *vars, int index) {
	// for(ListNode *i = list_begin(&globals); i != list_end(&globals); i = list_next(i)) {
	// 	Var *var = (Var*)i;
	// 	if(var->index == index) {
	// 		return var->object;
	// 	}
	// }
	if(globals[index]) {
		return *(double*)globals[index];
	}
	return vars[index];
}

int allocs = 0;

Object *new_object(Type type, int index) {
	Object *o = malloc(sizeof(Object));
	o->array = NULL;
	o->type = type;
	o->index = index;
	o->refs = 0;

	allocs++;

	return o;
}

void free_object(Object *object) {
	if(object->array) {
		free(object->array);
	}
	free(object);

	allocs--;

	// printf("OBJECTS STILL REFRENCED: %i\n\n", allocs);
}

int64_t eval(int pc) {
	/*
		assume if instance == NULL then method is static
	*/
	clock_t begin;

	/* stack */
	int64_t stack[16][1024];
	/* frame pointer */
	int64_t fp = 0;
	/* stack pointer */
	int64_t sp = 512;

	while(pc < code_size) {
		Op *current = codes[pc];
		switch(current->op) {
			case OP_LOAD: {
				if(globals[current->left]) {
					Object *var = globals[current->left];
					PUSH_STACK_OBJECT(var);
				} else {
					int64_t var = stack[fp][current->left];
					PUSH_STACK(var);
				}
			}
			break;
			case OP_STORE: {
				int64_t v1 = POP_STACK();
				stack[fp][current->left] = v1;
			}
			break;
			case OP_DUP: {
				int64_t a = POP_STACK();
				PUSH_STACK(a);
				PUSH_STACK(a);
			}
			break;
			case OP_PUSH: {
				PUSH_STACK((int64_t)current->left);
			}
			break;
			case OP_LOAD_CONST: {
				if(cache[(int)current->left]) {
					Object *o = cache[(int)current->left];
					
					PUSH_STACK_OBJECT(o);
				} else {
					Object *o = new_object(TY_ARRAY, FIND_OR_INSERT_CONST(constants, "char"));
				
					char *str = GET_CONST(current->left);
					int   size = strlen(str);

					o->array = malloc(sizeof(char) * size);

					for(int i = 0; i < size; i++) {
						o->array[i] = (char)str[i];
					}

					store_var_double(&o->varlist, FIND_OR_INSERT_CONST(constants, "count"), size);
					
					cache[(int)current->left] = o;

					PUSH_STACK_OBJECT(o);
				}
			}
			break;
			case OP_LOAD_FIELD: {
				Object *instance = POP_STACK_OBJECT();
				int64_t var = instance->varlist[current->left];

				PUSH_STACK(var);
			}
			break;
			case OP_STORE_FIELD: {
				Object *instance = POP_STACK_OBJECT();
				int64_t v2 = POP_STACK();
				instance->varlist[current->left] = v2;
			}
			break;
			case OP_POP: {
				POP_STACK();
			}
			break;
			case OP_CMPEQ: {
				int64_t a = POP_STACK();
				int64_t b = POP_STACK();
				int64_t c = b == a;
				PUSH_STACK(c);
			}
			break;
			case OP_CMPGT: {
				int64_t a = POP_STACK();
				int64_t b = POP_STACK();
				int64_t c = b > a;
				PUSH_STACK(c);
			}
			break;
			case OP_CMPLT: {
				int64_t a = POP_STACK();
				int64_t b = POP_STACK();
				int64_t c = b < a;
				PUSH_STACK(c);
			}
			break;
			case OP_ADD: {
				int64_t a = POP_STACK();
				int64_t b = POP_STACK();
				int64_t c = b + a;
				PUSH_STACK(c);
			}
			break;
			case OP_SUB: {
				int64_t a = POP_STACK();
				int64_t b = POP_STACK();
				int64_t c = b - a;
				PUSH_STACK(c);
			}
			break;
			case OP_MUL: {
				int64_t a = POP_STACK();
				int64_t b = POP_STACK();
				int64_t c = b * a;
				PUSH_STACK(c);
			}
			break;
			case OP_DIV: {
				int64_t a = POP_STACK();
				int64_t b = POP_STACK();
				int64_t c = b / a;
				PUSH_STACK(c);
			}
			break;
			case OP_FADD: {
				double a = POP_STACK();
				double b = POP_STACK();
				double c = b + a;
				int64_t d = *(int64_t*)&c;
				PUSH_STACK(d);
			}
			break;
			case OP_FSUB: {
				double a = POP_STACK();
				double b = POP_STACK();
				double c = b - a;
				int64_t d = *(int64_t*)&c;
				PUSH_STACK(d);
			}
			break;
			case OP_FMUL: {
				double a = POP_STACK();
				double b = POP_STACK();
				double c = b * a;
				int64_t d = *(int64_t*)&c;
				PUSH_STACK(d);
			}
			break;
			case OP_FDIV: {
				double a = POP_STACK();
				double b = POP_STACK();
				double c = b / a;
				int64_t d = *(int64_t*)&c;
				PUSH_STACK(d);
			}
			break;
			case OP_MOD: {
				int64_t a = (int)POP_STACK();
				int64_t b = POP_STACK();
				int64_t c = b % a;
				PUSH_STACK(c);
			}
			break;
			case OP_OR: {
				int64_t a = POP_STACK();
				int64_t b = POP_STACK();
				int64_t c = b || a;
				PUSH_STACK(c);
			}
			break;
			case OP_CALL: {
				int64_t arg_length = POP_STACK();

				int64_t args[arg_length];
				for(int i = 0; i < arg_length; i++) {
					int64_t arg = POP_STACK();
					args[i] = arg;
				}

				Object *instance = POP_STACK_OBJECT();

				PUSH_FRAME(); 

				stack[fp][FIND_OR_INSERT_CONST(constants, "this")] = instance;
				PUSH_STACK(pc);
				for(int i = 0; i < arg_length; i++) {
					PUSH_STACK(args[i]);
				}

				pc = (uint32_t)current->left - 1;

				continue;
			}
			break;
			case OP_SYSCALL: {
				int name = (int)POP_STACK();

				if(name == 1) {
					int64_t arg = POP_STACK();
					printf("%li\n", arg);

					PUSH_STACK(0);
				} else if(name == 2000) {
					Object *arg = POP_STACK_OBJECT();

					for(int i = 0; i < 32; i++) {
						printf("%02x\n", arg->array[i] & 0xff);
					}

					PUSH_STACK(0);
				} else if(name == 8000) {
					Object  *dst        = POP_STACK_OBJECT();
					int64_t  dst_offset = POP_STACK();
					Object  *src        = POP_STACK_OBJECT();
					int64_t  src_offset = POP_STACK();
					int64_t  b          = POP_STACK();

					memcpy(dst->array + dst_offset, src->array + src_offset, b);

					PUSH_STACK(0);
				} else if(name == 2) {
					int64_t arg = POP_STACK();
					printf("%c", (char)arg);

					PUSH_STACK(0);
				} else if(name == 5) {
					Object *arg = POP_STACK_OBJECT();
					free_object(arg);
					PUSH_STACK(0);
				} else if(name == 60) {
					int sockfd = socket(AF_INET, SOCK_STREAM, 0);

					setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

					PUSH_STACK(sockfd);
				} else if(name == 61) {
					int64_t fd   = POP_STACK();
					Object  *ip   = POP_STACK_OBJECT();
					int64_t port = POP_STACK();
					char ip_c[128];
					strncpy(ip_c, ip->array, ip->varlist[FIND_OR_INSERT_CONST(constants, "count")]);

					struct sockaddr_in servaddr;
					servaddr.sin_family = AF_INET;
					servaddr.sin_addr.s_addr = inet_addr(ip_c);
					servaddr.sin_port = htons((int)port);

					int result = bind((int)fd, (struct sockaddr*)&servaddr, sizeof(servaddr));
					listen((int)fd, 5);

					PUSH_STACK(result == 0);
				} 
				else if(name == 62) {
					int64_t fd = POP_STACK();

					int newfd = accept((int)fd, NULL, 0);

					PUSH_STACK(newfd);
				} else if(name == 63) {
					int64_t fd = POP_STACK();
					int64_t size = POP_STACK();

					Object *r = new_object(TY_ARRAY, FIND_OR_INSERT_CONST(constants, "char"));
					
					r->array = malloc(sizeof(char) * size);

					char data[8192];
					int actual = read((int)fd, r->array, size);

					store_var_double(&r->varlist, FIND_OR_INSERT_CONST(constants, "count"), actual);

					PUSH_STACK(r);
				} else if(name == 64) {
					int64_t fd = POP_STACK();
					Object *data = POP_STACK_OBJECT();

					int w = write((int)fd, data->array, data->varlist[FIND_OR_INSERT_CONST(constants, "count")]);

					PUSH_STACK(w);
				} else if(name == 65) {
					int64_t fd = POP_STACK();

					close((int)fd);

					PUSH_STACK(0);
				} else if(name == 13) {
					begin = clock();
				} else if(name == 14) {
					clock_t end = clock();
					double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
					printf("%f\n", time_spent);
				} else {
					printf("unknown syscall %i\n", name);
					exit(1);
				}
			}
			break;
			case OP_NEWO: {
				Object *o = new_object(TY_VARIABLE, current->left);
				PUSH_STACK_OBJECT(o);
			}
			break;
			case OP_NEWARRAY: {
				int64_t size = POP_STACK();

				Object *instance = new_object(TY_ARRAY, current->left);
				instance->array = malloc(sizeof(char) * size);

				memset(instance->array, 0, sizeof(char) * size);

				store_var_double(&instance->varlist, FIND_OR_INSERT_CONST(constants, "count"), size);

				PUSH_STACK_OBJECT(instance);
			}
			break;
			case OP_LOAD_ARRAY: {
				int64_t index = POP_STACK();
				Object *instance = POP_STACK_OBJECT();

				int64_t item = (int64_t)instance->array[index];

				PUSH_STACK(item);
			}
			break;
			case OP_STORE_ARRAY: {
				int64_t index = POP_STACK();
				Object *instance = POP_STACK_OBJECT();
				int64_t value = POP_STACK();

				instance->array[index] = (char)value;
			}
			break;
			case OP_JE: {
				int64_t a = POP_STACK();
				int64_t b = POP_STACK();

				if(a == b) {
					pc = current->left - 1;
					continue;
				}
			}
			break;
			case OP_JMP: {
				pc = current->left - 1;

				continue;
			}
			break;
			case OP_RET: {
				int64_t ret_data = POP_STACK();
				int64_t ret_addr = POP_STACK();

				POP_FRAME();

				PUSH_STACK(ret_data);

				pc = ret_addr + 1;

				continue;
			}
			break;
			default: {
				printf("illegal instruction %i\n", current->op);
				exit(1);
			}
			break;
		}
		pc++;
	}

	return 0;
}

void intepreter(const char *input) {
	/* code */
	signal(SIGPIPE, SIG_IGN);

	int entry = load_file(input);

	// emit_print();

	eval(entry - 1);
}