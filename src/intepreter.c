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

Object *cache[8192] = {}; // generate instances of all classes to allow static invoking
Object *globals[8192] = {};
static char *constants[8192] = {};
static List program;

void load_file(const char *name) {
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

	int class_count = 0;
	fread(&class_count, sizeof(class_count), 1, fp);

	for(int i = 0; i < class_count; i++) {
		short method_count = 0;
		short class_name = 0;
		fread(&method_count, sizeof(method_count), 1, fp);
		fread(&class_name, sizeof(class_name), 1, fp);

		Class *class = malloc(sizeof(Class));
		class->name = GET_CONST(class_name);
		class->index = class_name;
		list_clear(&class->method);

		for(int x = 0; x < method_count; x++) {
			short op_count = 0;
			short method_id = 0;
			fread(&op_count, sizeof(op_count), 1, fp);
			fread(&method_id, sizeof(method_id), 1, fp);

			Method *method = malloc(sizeof(Method));
			method->name = GET_CONST(method_id);
			method->index = method_id;
			method->code_count = op_count;
			
			method->codes = malloc(sizeof(Op *) * op_count);

			for(int y = 0; y < op_count; y++) {
				char     op = 0;
				int64_t op_left = 0;
				fread(&op, sizeof(op), 1, fp);
				fread(&op_left, sizeof(op_left), 1, fp);

				Op *ins = malloc(sizeof(Op));
				ins->op = op;
				ins->left = op_left;

				method->codes[y] = ins;
			}

			list_insert(list_end(&class->method), method);
		}

		list_insert(list_end(&program), class);
	}

	fclose(fp);
}

void emit_print() {
	for(ListNode *cn = list_begin(&program); cn != list_end(&program); cn = list_next(cn)) {
		Class *c = (Class*)cn;

		printf("@class %s\n", c->name);

		for(ListNode *mn = list_begin(&c->method); mn != list_end(&c->method); mn = list_next(mn)) {
			Method *m = (Method*)mn;

			int line = 1;
			printf("\t@method %s\n", m->name);
			printf("\tLINE\tOP\tVALUE\n\t--------------------------\n");

			for(int pc = 0; pc < m->code_count; pc++) {
				Op *ins = m->codes[pc];
				switch(ins->op) {
					case OP_LOAD: {
						printf("\t%i\tLOAD\t%s\n", line, GET_CONST(ins->left));
					}
					break;
					case OP_STORE: {
						printf("\t%i\tSTORE\t%s\n", line, GET_CONST(ins->left));
					}
					break;
					case OP_POP: {
						printf("\t%i\tPOP\t\n", line);
					}
					break;
					case OP_CMPEQ: {
						printf("\t%i\tCMPEQ\n", line);
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
					case OP_OR: {
						printf("\t%i\tOR\n", line);
					}
					break;
					case OP_LOAD_NUMBER: {
						printf("\t%i\tLOAD_NUMBER\t%li\n", line, ins->left);
					}
					break;
					case OP_LOAD_CONST: {
						printf("\t%i\tLOAD_CONST\t%i\t//%s\n", line, (int)ins->left, GET_CONST(ins->left));
					}
					break;
					case OP_LOAD_MEMBER: {
						printf("\t%i\tLOAD_MEMBER\t%s\n", line, GET_CONST(ins->left));
					}
					break;
					case OP_STORE_MEMBER: {
						printf("\t%i\tSTORE_MEMBER\t%s\n", line, GET_CONST(ins->left));
					}
					break;
					case OP_CALL: {
						printf("\t%i\tCALL\t\tARGLEN: %i\n", line, (int)ins->left);
					}
					break;
					case OP_SYSCALL: {
						printf("\t%i\tSYSCALL\t\tARGLEN: %i\n", line, (int)ins->left);
					}
					break;
					case OP_NEW: {
						printf("\t%i\tNEW\t\tARGLEN: %i\n", line, (int)ins->left);
					}
					break;
					case OP_NEWARRAY: {
						printf("\t%i\tNEWARRAY\t%s\n", line, GET_CONST(ins->left));
					}
					break;
					case OP_LOAD_ARRAY: {
						printf("\t%i\tLOAD_ARRAY\n", line);
					}
					break;
					case OP_STORE_ARRAY: {
						printf("\t%i\tSTORE_ARRAY\n", line);
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

Class *get_class(int index) {
	for(ListNode *i = list_begin(&program); i != list_end(&program); i = list_next(i)) {
		Class *class = (Class*)i;
		if(index == class->index) {
			return class;
		}
	}
	return NULL;
}

Method *get_method(int name1, int name2) {
	Class *class = get_class(name1);
	if(class) {
		for(ListNode *i = list_begin(&class->method); i != list_end(&class->method); i = list_next(i)) {
			Method *method = (Method*)i;
			if(name2 == method->index) {
				return method;
			}
		}
	}
	return NULL;
}

int allocs = 0;

Object *new_object(Type type, int index) {
	Object *o = malloc(sizeof(Object));
	o->array = NULL;
	o->bound = NULL;
	o->type = type;
	o->index = index;
	o->refs = 0;

	allocs++;

	if(type == TY_VARIABLE) {
		Class *skeleton = get_class(index);
		if(!skeleton) {
			printf("Error, class %s not defined\n", GET_CONST(index));
			exit(1);
		}

		for(ListNode *i = list_begin(&skeleton->method); i != list_end(&skeleton->method); i = list_next(i)) {
			Method *method = (Method*)i;
			Object *var = new_object(TY_FUNCTION, method->index);
			var->bound = o;
			var->method = method;
			store_var(&o->varlist, method->index, var);
		}
	}

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

int64_t eval(Object *instance, Method *method, int64_t *args, int args_length) {
	/*
		assume if instance == NULL then method is static
	*/
	int64_t ret = 0;

	clock_t begin;

	int64_t varlist[2048];

	if(instance != NULL) {
		store_var(varlist, FIND_OR_INSERT_CONST(constants, "this"), instance);
	}

	int64_t stack[512];
	int sp = 0;

	if(args) {
		for(int i = 0; i < args_length; i++) {
			int64_t arg = args[i];
			PUSH_STACK(arg);
		}
	}

	int pc = 0;
	while(pc < method->code_count) {
		Op *current = method->codes[pc];
		switch(current->op) {
			case OP_LOAD: {
				if(globals[current->left]) {
					Object *var = globals[current->left];
					PUSH_STACK_OBJECT(var);
				} else {
					int64_t var = varlist[current->left];
					PUSH_STACK(var);
				}
			}
			break;
			case OP_STORE: {
				int64_t v1 = POP_STACK();
				varlist[current->left] = v1;
			}
			break;
			case OP_LOAD_NUMBER: {
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
			case OP_LOAD_MEMBER: {
				Object *instance = POP_STACK_OBJECT();
				int64_t var = instance->varlist[current->left];

				PUSH_STACK(var);
			}
			break;
			case OP_STORE_MEMBER: {
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
				Object *instance = POP_STACK_OBJECT();

				int64_t args[(int)current->left];

				for(int i = 0; i < current->left; i++) {
					int64_t arg = POP_STACK();
					args[i] = arg;
				}

				if(instance->type != TY_FUNCTION) {
					printf("unknown function call %s\n", GET_CONST(instance->index));
					exit(1);
				}

				int64_t r = eval(instance->bound, instance->method, args, (int)current->left);

				PUSH_STACK(r);
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
					Object   *dst        = POP_STACK_OBJECT();
					int64_t  dst_offset = POP_STACK();
					Object   *src        = POP_STACK_OBJECT();
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
			case OP_NEW: {
				Object *o = new_object(TY_VARIABLE, current->left);
				PUSH_STACK_OBJECT(o);
			}
			break;
			case OP_NEWARRAY: {
				int64_t size = POP_STACK();

				Object *instance = new_object(TY_ARRAY, current->left);
				instance->array = malloc(sizeof(char) * size);

				for(int i = 0; i < size; i++) {
					instance->array[i] = 0;
				}

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
			case OP_JMPIFT: {
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
				ret = POP_STACK();

				goto cleanup;
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

	cleanup:;

	return ret;
}

void intepreter(const char *input) {
	/* code */
	signal(SIGPIPE, SIG_IGN);

	list_clear(&program);

	load_file(input);

	emit_print();

	for(ListNode *cn = list_begin(&program); cn != list_end(&program); cn = list_next(cn)) {
		Class *c = (Class*)cn;

		Object *var = new_object(TY_VARIABLE, c->index);
		// store_var(&globals, c->index, var);
		globals[c->index] = var;
	}

	Method *method_main = get_method(FIND_OR_INSERT_CONST(constants, "Main"), FIND_OR_INSERT_CONST(constants, "main"));
	if(!method_main) {
		printf("entry point method main not found\n");
		exit(1);
	}

	eval(NULL, method_main, NULL, 0);
}