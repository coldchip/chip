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
#include "optimize.h"
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
		uint8_t  encoded_op = 0;
		fread(&encoded_op, sizeof(encoded_op), 1, fp);

		uint8_t op      = (encoded_op >> 2) & 0x3F;
		uint8_t width   = (encoded_op >> 0) & 0x03;
		int64_t op_left = 0;

		if(op_size[op]) {
			fread(&op_left, sizeof(char), 1 << width, fp);
		}

		Op *ins = malloc(sizeof(Op));
		ins->op = op;
		ins->left = op_left;

		codes[code_size++] = ins;
	}

	fclose(fp);

	return entry;
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

Object *new_object(int size) {
	Object *o = malloc(sizeof(Object));
	o->varlist = malloc(sizeof(Slot) * size);
	o->array = NULL;

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
	clock_t begin;

	/* stack */
	Slot stack[128][1024] = {};
	/* frame pointer */
	int64_t fp = 0;
	/* stack pointer */
	int64_t sp = 512;

	while(pc < code_size) {
		Op *current = codes[pc];
		switch(current->op) {
			case OP_NOP: {

			}
			break;
			case OP_LOAD: {
				Slot var = GET_VAR_SLOT(current->left);
				PUSH_STACK_SLOT(var);
			}
			break;
			case OP_STORE: {
				Slot var = POP_STACK_SLOT();
				SET_VAR_SLOT(current->left, var);
			}
			break;
			case OP_DUP: {
				Slot s = TOP_STACK_SLOT();
				PUSH_STACK_SLOT(s);
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
					Object *o = new_object(1);
				
					char *str = GET_CONST(current->left);
					int   size = strlen(str);

					o->array = malloc(sizeof(char) * size);

					for(int i = 0; i < size; i++) {
						o->array[i] = (char)str[i];
					}

					o->varlist[0].value = size;
					
					cache[(int)current->left] = o;

					PUSH_STACK_OBJECT(o);
				}
			}
			break;
			case OP_LOAD_FIELD: {
				Object *instance = POP_STACK_OBJECT();
				Slot var = instance->varlist[current->left];
				PUSH_STACK_SLOT(var);
			}
			break;
			case OP_STORE_FIELD: {
				Object *instance = POP_STACK_OBJECT();
				Slot var = POP_STACK_SLOT();
				instance->varlist[current->left] = var;
			}
			break;
			case OP_POP: {
				POP_STACK_OBJECT();
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
			case OP_NEG: {
				int64_t a = POP_STACK();
				PUSH_STACK(-a);
			}
			break;
			case OP_MOD: {
				int64_t a = (int)POP_STACK();
				int64_t b = POP_STACK();
				int64_t c = b % a;
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

				Slot instance = POP_STACK_SLOT();

				PUSH_FRAME(); 

				// preserve SP

				SET_VAR_SLOT(0, instance);

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

					strncpy(ip_c, ip->array, ip->varlist[0].value);


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
					Object *buffer = POP_STACK_OBJECT();
					int64_t size = POP_STACK();

					int r = read((int)fd, buffer->array, size);

					PUSH_STACK(r);
				} else if(name == 64) {
					int64_t fd = POP_STACK();
					Object *data = POP_STACK_OBJECT();
					int64_t size = POP_STACK();

					int w = write((int)fd, data->array, size);

					PUSH_STACK(w);
				} else if(name == 65) {
					int64_t fd = POP_STACK();

					close((int)fd);

					PUSH_STACK(0);
				} else if(name == 13) {
					begin = clock();
				} else if(name == 6969) {
					exit(0);
				} else if(name == 14) {
					clock_t end = clock();
					double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
					printf("%f\n", time_spent);
				} else if(name == 33) {
					PUSH_STACK(rand());
				} else if(name == 34555) {
					for(int x = 0; x < 128; x++) {
						Slot *f = stack[x];
						for(int y = 0; y < 1024; y++) {
							Slot s = f[y];
							if(s.is_ref) {
								printf("%p\n", s.ref);
							}
						}
					}
					PUSH_STACK(0);
				} else {
					printf("unknown syscall %i\n", name);
					exit(1);
				}
			}
			break;
			case OP_NEWO: {
				Object *o = new_object((int)current->left);
				PUSH_STACK_OBJECT(o);
			}
			break;
			case OP_NEW_ARRAY: {
				int64_t size = POP_STACK();

				Object *instance = new_object(1);
				instance->array = malloc(sizeof(char) * size);

				memset(instance->array, 0, sizeof(char) * size);

				instance->varlist[0].value = size;

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
			case OP_JNE: {
				int64_t a = POP_STACK();
				int64_t b = POP_STACK();

				if(a != b) {
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
				Slot ret_data = POP_STACK_SLOT();
				int64_t ret_addr = POP_STACK();

				POP_FRAME();

				PUSH_STACK_SLOT(ret_data);

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

	eval(entry - 1);
}