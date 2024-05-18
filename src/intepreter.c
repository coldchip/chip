/*
	The Chip Language Intepreter
	@Copyright Ryan Loh 2023
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <math.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include "chip.h"
#include "list.h"
#include "optimize.h"
#include "intepreter.h"

List objects;

static char *constants[8192] = {};
static char codes[32768] = {};
int code_size = 0;

int load_file(const char *name) {
	FILE *fp = fopen(name, "rb");
	if(!fp) {
		printf("unable to load file %s\n", name);
		exit(1);
	}

	chip_hdr_t hdr;
	if(fread(&hdr, sizeof(hdr), 1, fp) != 1) {
		printf("unable to read file\n");
		exit(1);
	}

	if(NTOHL(hdr.version) != CHIP_VERSION) {
		printf("incorrect chip executable version\n");
		exit(1);
	}

	for(int i = 0; i < NTOHLL(hdr.code_size); i++) {
		if(fread(&codes[code_size], sizeof(char), 1, fp) != 1) {
			printf("unable to read file\n");
			exit(1);
		}
		code_size++;
	}

	for(int z = 0; z < NTOHLL(hdr.const_size); z++) {
		int constant_length = 0;
		if(fread(&constant_length, sizeof(constant_length), 1, fp) != 1) {
			printf("unable to read file\n");
			exit(1);
		}

		char constant_data[constant_length + 1];
		memset(constant_data, 0, sizeof(constant_data));

		if(fread(&constant_data, sizeof(char), constant_length, fp) != constant_length) {
			printf("unable to read file\n");
			exit(1);
		}

		constant_data[sizeof(constant_data) - 1] = '\0';

		SET_CONST(z, strdup(constant_data));
	}

	fclose(fp);

	return NTOHLL(hdr.entry);
}

int allocs = 0;

Object *new_object(int size) {
	Object *o = malloc(sizeof(Object));
	o->array = NULL;
	o->varlist = malloc(sizeof(Slot) * size);
	o->size = size;
	o->type = 1;
	o->is_marked = false;

	for(int x = 0; x < size; x++) {
		o->varlist[x].is_ref = false;
	}

	list_insert(list_end(&objects), o);

	allocs++;

	return o;
}

void free_object(Object *object) {
	if(object->array) {
		free(object->array);
	}

	free(object->varlist);

	list_remove(&object->node);
	free(object);

	allocs--;

	// printf("OBJECTS STILL REFRENCED: %i %li\n\n", allocs, list_size(&objects));
}

void gc(Slot *stack, int size) {
	mark(stack, size);
	sweep();
}

void mark(Slot *stack, int size) {
	for(int y = 0; y < size; y++) {
		Slot s = stack[y];
		if(s.is_ref) {
			Object *o = s.ref;
			if(!o->is_marked) {
				o->is_marked = true;
				mark(o->varlist, o->size);
			}
		}
	}
}

void sweep() {
	ListNode *i = list_begin(&objects);
	while(i != list_end(&objects)) {
		Object *object = (Object*)i;
		i = list_next(i);

		if(!object->is_marked) {
			free_object(object);
		}

		object->is_marked = false;
	}
}

int64_t eval(int pc) {
	clock_t begin;

	/* stack */
	Slot stack[128 * 1024] = {0};
	/* var pointer */
	int64_t vp = 0;
	/* stack pointer */
	int64_t sp = 65535;

	while(pc < code_size) {
		uint8_t op      = (codes[pc] >> 2) & 0x3F;
		uint8_t width   = (codes[pc] >> 0) & 0x03;
		int64_t left    = 0;

		pc++;

		if(op_size[op]) {
			left = *(int64_t*)&codes[pc];
			if(width == 0) {
				left = left & 0xFF;
			}
			if(width == 1) {
				left = NTOHS(left & 0xFFFF);
			}
			if(width == 2) {
				left = NTOHL(left & 0xFFFFFFFF);
			}
			if(width == 3) {
				left = NTOHLL(left & 0xFFFFFFFFFFFFFFFF);
			}
			pc += 1 << width;
		}

		switch(op) {
			case OP_NOP: {

			}
			break;
			case OP_LOAD: {
				Slot var = GET_VAR_SLOT(left);
				PUSH_STACK_SLOT(var);
			}
			break;
			case OP_STORE: {
				Slot var = POP_STACK_SLOT();
				SET_VAR_SLOT(left, var);
			}
			break;
			case OP_DUP: {
				Slot s = TOP_STACK_SLOT();
				PUSH_STACK_SLOT(s);
			}
			break;
			case OP_PUSH: {
				PUSH_STACK((int64_t)left);
			}
			break;
			case OP_LOAD_CONST: {			
				char *str  = GET_CONST(left);
				int   size = strlen(str);

				Object *o = new_object(size + 1);
				o->array = malloc(sizeof(char) * size);

				for(int i = 0; i < size; i++) {
					o->array[i] = (char)str[i];
				}

				o->varlist[0].is_ref = false;
				o->varlist[0].value  = size;

				PUSH_STACK_OBJECT(o);
			}
			break;
			case OP_LOAD_FIELD: {
				Object *instance = POP_STACK_OBJECT();
				Slot var = instance->varlist[left];
				PUSH_STACK_SLOT(var);
			}
			break;
			case OP_STORE_FIELD: {
				Object *instance = POP_STACK_OBJECT();
				Slot var = POP_STACK_SLOT();
				instance->varlist[left] = var;
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
			case OP_SHR: {
				int64_t a = POP_STACK();
				int64_t b = POP_STACK();
				int64_t c = b >> a;
				PUSH_STACK(c);
			}
			break;
			case OP_SHL: {
				int64_t a = POP_STACK();
				int64_t b = POP_STACK();
				int64_t c = b << a;
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
			case OP_NOT: {
				int64_t a = POP_STACK();
				PUSH_STACK(~a);
			}
			break;
			case OP_OR: {
				int64_t a = POP_STACK();
				int64_t b = POP_STACK();
				PUSH_STACK(a | b);
			}
			break;
			case OP_XOR: {
				int64_t a = POP_STACK();
				int64_t b = POP_STACK();
				PUSH_STACK(a ^ b);
			}
			break;
			case OP_AND: {
				int64_t a = POP_STACK();
				int64_t b = POP_STACK();
				PUSH_STACK(a & b);
			}
			break;
			case OP_MOD: {
				int64_t a = POP_STACK();
				int64_t b = POP_STACK();
				int64_t c = b % a;
				PUSH_STACK(c);
			}
			break;
			case OP_FADD: {
				double a = POP_STACK_DOUBLE();
				double b = POP_STACK_DOUBLE();
				double c = b + a;
				PUSH_STACK_DOUBLE(c);
			}
			break;
			case OP_FSUB: {
				double a = POP_STACK_DOUBLE();
				double b = POP_STACK_DOUBLE();
				double c = b - a;
				PUSH_STACK_DOUBLE(c);
			}
			break;
			case OP_FMUL: {
				double a = POP_STACK_DOUBLE();
				double b = POP_STACK_DOUBLE();
				double c = b * a;
				PUSH_STACK_DOUBLE(c);
			}
			break;
			case OP_FDIV: {
				double a = POP_STACK_DOUBLE();
				double b = POP_STACK_DOUBLE();
				double c = b / a;
				PUSH_STACK_DOUBLE(c);
			}
			break;
			case OP_FNEG: {
				double a = POP_STACK_DOUBLE();
				PUSH_STACK_DOUBLE(-a);
			}
			break;
			case OP_FMOD: {
				double a = POP_STACK_DOUBLE();
				double b = POP_STACK_DOUBLE();
				double c = fmod(a, b);
				PUSH_STACK_DOUBLE(c);
			}
			break;
			case OP_I2F: {
				int64_t a = POP_STACK();
				double  b = (double)a;
				PUSH_STACK_DOUBLE(b);
			}
			break;
			case OP_CALL: {
				int64_t arg_length = POP_STACK();

				Slot args[arg_length];
				for(int i = 0; i < arg_length; i++) {
					args[i] = POP_STACK_SLOT();
				}
				Slot instance = POP_STACK_SLOT();

				PUSH_FRAME(); 

				PUSH_STACK(pc);

				PUSH_STACK_SLOT(instance);
				for(int i = 0; i < arg_length; i++) {
					PUSH_STACK_SLOT(args[i]);
				}

				pc = (uint32_t)left;

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

					memcpy(dst->array + (dst_offset * 8), src->array + (src_offset * 8), b * 8);

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
					Object *buffer = POP_STACK_OBJECT();
					int64_t size = POP_STACK();

					int w = write((int)fd, buffer->array, size);

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
				} else if(name == 49935) {
					double  number = POP_STACK_DOUBLE();
					Object *buffer    = POP_STACK_OBJECT();
					int length = sprintf(buffer->array, "%f", number);
					PUSH_STACK(length);
				} else if(name == 34569) {
					Object *buffer = POP_STACK_OBJECT();
					int64_t size = POP_STACK();

					int r = read(STDIN_FILENO, buffer->array, size);

					PUSH_STACK(r - 1);
				} else if(name == 34555) {
					gc(stack, 128 * 1024);
					PUSH_STACK(0);
				} else {
					printf("unknown syscall %i\n", name);
					exit(1);
				}
			}
			break;
			case OP_NEWO: {
				Object *o = new_object((int)left);
				PUSH_STACK_OBJECT(o);
			}
			break;
			case OP_NEW_ARRAY: {
				int64_t size = POP_STACK();

				size *= 8;

				Object *instance = new_object(size);
				instance->type = 54;


				instance->array = malloc(sizeof(char) * size);

				memset(instance->array, 0, sizeof(char) * size);

				instance->varlist[0].is_ref = false;
				instance->varlist[0].value  = (size / 8);

				PUSH_STACK_OBJECT(instance);
			}
			break;
			case OP_LOAD_ARRAY: {
				int64_t index    = POP_STACK();
				Object *instance = POP_STACK_OBJECT();

				if(instance->type == 54) {
					if(index > (instance->size / 8) - 1) {
						printf("array out of bound access read error %li %i\n", index, instance->size - 1);
						exit(1);
					}

					int64_t item = (int64_t)instance->array_i[index];
					PUSH_STACK(item);
				} else {
					if(index > instance->size - 1) {
						printf("array out of bound access read error %li %i\n", index, instance->size - 1);
						exit(1);
					}

					int64_t item = (char)instance->array[index];
					PUSH_STACK(item);
				}
			}
			break;
			case OP_STORE_ARRAY: {
				int64_t index = POP_STACK();
				Object *instance = POP_STACK_OBJECT();
				int64_t value = POP_STACK();

				if(instance->type == 54) {
					if(index > (instance->size / 8) - 1) {
						printf("array out of bound access write error %li %i\n", index, instance->size - 1);
						exit(1);
					}

					instance->array_i[index] = value;
				} else {
					if(index > instance->size - 1) {
						printf("array out of bound access write error %li %i\n", index, instance->size - 1);
						exit(1);
					}

					instance->array[index] = (char)value;
				}
			}
			break;
			case OP_JE: {
				int64_t a = POP_STACK();
				int64_t b = POP_STACK();

				if(a == b) {
					pc = left;
					continue;
				}
			}
			break;
			case OP_JMP: {
				pc = left;
				continue;
			}
			break;
			case OP_RET: {
				Slot ret_data = POP_STACK_SLOT();
				int64_t ret_addr = POP_STACK();

				POP_FRAME();

				PUSH_STACK_SLOT(ret_data);

				pc = ret_addr;

				continue;
			}
			break;
			case OP_HALT: {
				int64_t ret_code = POP_STACK();
				exit(ret_code);
			}
			break;
			default: {
				printf("illegal instruction %i\n", op);
				exit(1);
			}
			break;
		}
	}

	return 0;
}

void intepreter(const char *input) {
	/* code */
	signal(SIGPIPE, SIG_IGN);

	list_clear(&objects);

	uint64_t entry = load_file(input);

	eval(entry);
}