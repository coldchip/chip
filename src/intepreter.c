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
#include "chip.h"

List globals; // generate instances of all classes to allow static invoking
static char *constants[8192] = {};
static List program;
Object *cache[8192] = {};

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
		list_clear(&class->method);

		for(int x = 0; x < method_count; x++) {
			short op_count = 0;
			short method_id = 0;
			fread(&op_count, sizeof(op_count), 1, fp);
			fread(&method_id, sizeof(method_id), 1, fp);

			Method *method = malloc(sizeof(Method));
			method->name = GET_CONST(method_id);
			list_clear(&method->op);

			for(int y = 0; y < op_count; y++) {
				char   op = 0;
				double op_left = 0;
				fread(&op, sizeof(op), 1, fp);
				fread(&op_left, sizeof(op_left), 1, fp);

				Op *ins = malloc(sizeof(Op));
				ins->op = op;
				ins->left = op_left;

				list_insert(list_end(&method->op), ins);
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

			for(ListNode *op = list_begin(&m->op); op != list_end(&m->op); op = list_next(op)) {
				Op *ins = (Op*)op;
				switch(ins->op) {
					case OP_LOAD_VAR: {
						printf("\t%i\tLOAD_VAR\t%s\n", line, GET_CONST(ins->left));
					}
					break;
					case OP_STORE_VAR: {
						printf("\t%i\tSTORE_VAR\t%s\n", line, GET_CONST(ins->left));
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
					case OP_LOAD_NUMBER: {
						printf("\t%i\tLOAD_NUMBER\t%f\n", line, ins->left);
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
						printf("\t%i\tCALL\tARGLEN: %i\n", line, (int)ins->left);
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
						printf("\t%i\tNEWARRAY\n", line);
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

Op *op_at(List *program, int line) {
	int size = 1;
	ListNode *position;

	for(position = list_begin(program); position != list_end(program); position = list_next(position)) {
		if(size == line) {
			return (Op*)position;
		}
		size++;
	}

	return NULL;
}

void store_var(List *vars, char *name, Object *object) {
	Var *previous = load_var(vars, name);
	if(previous) {
		free_var(previous);
	}

	Var *var = malloc(sizeof(Var));
	strcpy(var->name, name);
	var->object = object;
	INCREF(object);
	list_insert(list_end(vars), var);
}

Var *load_var(List *vars, char *name) {
	for(ListNode *i = list_begin(&globals); i != list_end(&globals); i = list_next(i)) {
		Var *var = (Var*)i;
		if(strcmp(name, var->name) == 0) {
			return var;
		}
	}

	for(ListNode *i = list_begin(vars); i != list_end(vars); i = list_next(i)) {
		Var *var = (Var*)i;
		if(strcmp(name, var->name) == 0) {
			return var;
		}
	}
	return NULL;
}

void free_var(Var *var) {
	DECREF(var->object);
	list_remove(&var->node);
	free(var);
}

Class *get_class(char *name) {
	for(ListNode *i = list_begin(&program); i != list_end(&program); i = list_next(i)) {
		Class *class = (Class*)i;
		if(strcmp(name, class->name) == 0) {
			return class;
		}
	}
	return NULL;
}

Method *get_method(char *name1, char *name2) {
	Class *class = get_class(name1);
	if(class) {
		for(ListNode *i = list_begin(&class->method); i != list_end(&class->method); i = list_next(i)) {
			Method *method = (Method*)i;
			if(strcmp(name2, method->name) == 0) {
				return method;
			}
		}
	}
	return NULL;
}

int allocs = 0;

Object *new_object(Type type, char *name) {
	Object *o = malloc(sizeof(Object));
	o->data_string = NULL;
	o->data_number = 0;
	o->array = NULL;
	o->bound = NULL;
	o->type = type;
	o->name = strdup(name);
	o->refs = 0;
	list_clear(&o->varlist);

	allocs++;

	if(type != TY_FUNCTION) {
		Class *skeleton = get_class(name);
		if(!skeleton) {
			printf("Error, class %s not defined\n", name);
			exit(1);
		}

		for(ListNode *i = list_begin(&skeleton->method); i != list_end(&skeleton->method); i = list_next(i)) {
			Method *method = (Method*)i;
			Object *var = new_object(TY_FUNCTION, method->name);
			var->bound = o;
			var->method = method;
			store_var(&o->varlist, method->name, var);
		}
	}

	return o;
}

void incref_object(Object *object) {
	object->refs++;
}

void decref_object(Object *object) {
	object->refs--;

	if(!object->refs > 0) {
		free_object(object);
	}
}

void free_object(Object *object) {
	while(!list_empty(&object->varlist)) {
		Var *var = (Var*)list_remove(list_begin(&object->varlist));
		free_var(var);
	}

	if(object->data_string) {
		free(object->data_string);
	}
	if(object->array) {
		free(object->array);
	}
	free(object->name);
	free(object);

	allocs--;

	// printf("OBJECTS STILL REFRENCED: %i\n\n", allocs);
}

Object *empty_return = NULL;

Object *eval(Object *instance, Method *method, List *args) {
	/*
		assume if instance == NULL then method is static
	*/
	List varlist;
	list_clear(&varlist);

	if(instance != NULL) {
		store_var(&varlist, "this", instance);
	}
	
	Op *current = (Op*)list_begin(&method->op);

	Object *stack[512];
	int sp = 0;

	Object *ret = empty_return;
	INCREF(ret);

	if(args) {
		while(!list_empty(args)) {
			Object *arg = (Object*)list_remove(list_begin(args));
			PUSH_STACK(arg);
		}
	}

	while(current != (Op*)list_end(&method->op)) {
		switch(current->op) {
			case OP_LOAD_VAR: {
				Var *var = load_var(&varlist, GET_CONST(current->left));
				if(!var) {
					printf("unable to load variable %s as it is not found\n", GET_CONST(current->left));
					exit(1);
				}

				Object *v1 = var->object;
				
				PUSH_STACK(v1);
				INCREF(v1);
			}
			break;
			case OP_STORE_VAR: {
				Object *v1 = POP_STACK();
				store_var(&varlist, GET_CONST(current->left), v1);

				DECREF(v1);
			}
			break;
			case OP_LOAD_NUMBER: {
				Object *o = new_object(TY_VARIABLE, "Number");
				o->data_number = current->left;
				
				PUSH_STACK(o);
				INCREF(o);
			}
			break;
			case OP_LOAD_CONST: {
				if(cache[(int)current->left]) {
					Object *o = cache[(int)current->left];
					
					PUSH_STACK(o);
					INCREF(o);
				} else {
					Object *o = new_object(TY_VARIABLE, "String");
					o->data_string = strdup(GET_CONST(current->left));
					cache[(int)current->left] = o;
					INCREF(o);
					
					PUSH_STACK(o);
					INCREF(o);
				}
			}
			break;
			case OP_LOAD_MEMBER: {
				Object *instance = POP_STACK();

				Var *var = load_var(&instance->varlist, GET_CONST(current->left));
				if(!var) {
					printf("Unknown variable member %s\n", GET_CONST(current->left));
					exit(1);
				}

				Object *v1 = var->object;
				
				PUSH_STACK(v1);

				DECREF(instance);
				INCREF(v1);
			}
			break;
			case OP_STORE_MEMBER: {
				Object *instance = POP_STACK();
				Object *v2 = POP_STACK();

				store_var(&instance->varlist, GET_CONST(current->left), v2);

				DECREF(instance);
				DECREF(v2);
			}
			break;
			case OP_POP: {
				DECREF(POP_STACK());
			}
			break;
			case OP_CMPEQ: {
				Object *v1 = POP_STACK();
				Object *v2 = POP_STACK();

				Object *v3 = new_object(TY_VARIABLE, "Number");

				if(strcmp(v1->name, "Number") == 0 && strcmp(v2->name, "Number") == 0) {
					v3->data_number = v2->data_number == v1->data_number;
				} else {
					v3->data_number = v2 == v1;
				}

				PUSH_STACK(v3);

				DECREF(v1);
				DECREF(v2);
				INCREF(v3);
			}
			break;
			case OP_CMPGT: {
				Object *v1 = POP_STACK();
				Object *v2 = POP_STACK();

				Object *v3 = new_object(TY_VARIABLE, "Number");
				v3->data_number = (int)v2->data_number > (int)v1->data_number;

				PUSH_STACK(v3);

				DECREF(v1);
				DECREF(v2);
				INCREF(v3);
			}
			break;
			case OP_CMPLT: {
				Object *v1 = POP_STACK();
				Object *v2 = POP_STACK();

				Object *v3 = new_object(TY_VARIABLE, "Number");
				v3->data_number = (int)v2->data_number < (int)v1->data_number;

				PUSH_STACK(v3);

				DECREF(v1);
				DECREF(v2);
				INCREF(v3);
			}
			break;
			case OP_ADD: {
				Object *v1 = POP_STACK();
				Object *v2 = POP_STACK();

				if(strcmp(v1->name, "Number") == 0 && strcmp(v2->name, "Number") == 0) {
					Object *v3 = new_object(TY_VARIABLE, "Number");
					v3->data_number = v2->data_number + v1->data_number;
					
					
					PUSH_STACK(v3);

					INCREF(v3);
				} else if(strcmp(v1->name, "String") == 0 && strcmp(v2->name, "String") == 0) {
					int sl1 = strlen(v1->data_string);
					int sl2 = strlen(v2->data_string);

					char *s = malloc(sl1 + sl2 + 1);
					memcpy(s, v2->data_string, sl2);
					memcpy(s + sl2, v1->data_string, sl1);

					s[sl1 + sl2] = '\0';

					Object *v3 = new_object(TY_VARIABLE, "String");
					v3->data_string = s;

					PUSH_STACK(v3);

					INCREF(v3);
				} else {
					printf("Unable to binary add unknown types [%s and %s]\n", v1->name, v2->name);
					exit(1);
				}

				DECREF(v1);
				DECREF(v2);
			}
			break;
			case OP_SUB: {
				Object *v1 = POP_STACK();
				Object *v2 = POP_STACK();

				Object *v3 = new_object(TY_VARIABLE, "Number");
				v3->data_number = v2->data_number - v1->data_number;
				
				PUSH_STACK(v3);

				DECREF(v1);
				DECREF(v2);
				INCREF(v3);
			}
			break;
			case OP_MUL: {
				Object *v1 = POP_STACK();
				Object *v2 = POP_STACK();

				Object *v3 = new_object(TY_VARIABLE, "Number");
				v3->data_number = v2->data_number * v1->data_number;
				
				PUSH_STACK(v3);

				DECREF(v1);
				DECREF(v2);
				INCREF(v3);
			}
			break;
			case OP_DIV: {
				Object *v1 = POP_STACK();
				Object *v2 = POP_STACK();

				Object *v3 = new_object(TY_VARIABLE, "Number");
				v3->data_number = v2->data_number / v1->data_number;
				
				PUSH_STACK(v3);

				DECREF(v1);
				DECREF(v2);
				INCREF(v3);
			}
			break;
			case OP_MOD: {
				Object *v1 = POP_STACK();
				Object *v2 = POP_STACK();

				Object *v3 = new_object(TY_VARIABLE, "Number");
				v3->data_number = ((int)v2->data_number) % ((int)v1->data_number);
				
				PUSH_STACK(v3);

				DECREF(v1);
				DECREF(v2);
				INCREF(v3);
			}
			break;
			case OP_CALL: {
				Object *instance = POP_STACK();

				List args;
				list_clear(&args);

				for(int i = 0; i < current->left; i++) {
					Object *arg = POP_STACK();

					list_insert(list_end(&args), arg);
				}

				if(instance->type != TY_FUNCTION) {
					printf("unknown function call %s\n", instance->data_string);
					exit(1);
				}

				Object *r = eval(instance->bound, instance->method, &args);
				PUSH_STACK(r);

				DECREF(instance);
			}
			break;
			case OP_SYSCALL: {
				Object *name = POP_STACK();

				if(strcmp(name->data_string, "console.read") == 0) {
					Object *text = POP_STACK();


					char buffer[8192];
					printf("%s", text->data_string);
					scanf("%s", buffer);

					Object *r = new_object(TY_VARIABLE, "String");
					r->data_string = strdup(buffer);

					PUSH_STACK(r);

					DECREF(text);
					INCREF(r);
				} else if(strcmp(name->data_string, "console.write") == 0) {
					Object *arg = POP_STACK();
					if(strcmp(arg->name, "String") == 0) {
						printf("%s", arg->data_string);
					} else if(strcmp(arg->name, "Number") == 0) {
						printf("%f", arg->data_number);
					} else {
						printf("%s@%p", arg->name, arg);
					}

					PUSH_STACK(empty_return);

					DECREF(arg);
					INCREF(empty_return);
				} else if(strcmp(name->data_string, "socket") == 0) {
					int sockfd = socket(AF_INET, SOCK_STREAM, 0);

					setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

					Object *r = new_object(TY_VARIABLE, "Number");
					r->data_number = sockfd;

					PUSH_STACK(r);

					INCREF(r);
				} else if(strcmp(name->data_string, "bind") == 0) {
					Object *fd   = POP_STACK();
					Object *ip   = POP_STACK();
					Object *port = POP_STACK();

					struct sockaddr_in servaddr;
					servaddr.sin_family = AF_INET;
					servaddr.sin_addr.s_addr = inet_addr(ip->data_string);
					servaddr.sin_port = htons((int)port->data_number);

					int result = bind((int)fd->data_number, (struct sockaddr*)&servaddr, sizeof(servaddr));
					listen((int)fd->data_number, 5);

					Object *r1 = new_object(TY_VARIABLE, "Number");
					r1->data_number = result == 0;

					PUSH_STACK(r1);

					DECREF(fd);
					DECREF(ip);
					DECREF(port);
					INCREF(r1);
				} else if(strcmp(name->data_string, "accept") == 0) {
					Object *fd = POP_STACK();

					int newfd = accept((int)fd->data_number, NULL, 0);

					Object *r = new_object(TY_VARIABLE, "Number");
					r->data_number = newfd;

					PUSH_STACK(r);

					DECREF(fd);
					INCREF(r);
				} else if(strcmp(name->data_string, "read") == 0) {
					Object *fd = POP_STACK();

					char data[8192];
					read((int)fd->data_number, data, sizeof(data));

					Object *r = new_object(TY_VARIABLE, "String");
					r->data_string = strdup(data);

					PUSH_STACK(r);

					DECREF(fd);
					INCREF(r);
				} else if(strcmp(name->data_string, "write") == 0) {
					Object *fd = POP_STACK();
					Object *data = POP_STACK();
					Object *length = POP_STACK();

					int w = write((int)fd->data_number, data->data_string, (int)length->data_number);
					Object *r1 = new_object(TY_VARIABLE, "Number");
					r1->data_number = w;

					PUSH_STACK(r1);

					DECREF(fd);
					DECREF(data);
					DECREF(length);
					INCREF(r1);
				} else if(strcmp(name->data_string, "close") == 0) {
					Object *fd = POP_STACK();

					close((int)fd->data_number);

					PUSH_STACK(empty_return);

					DECREF(fd);
					INCREF(empty_return);
				} else if(strcmp(name->data_string, "rand") == 0) {
					Object *r1 = new_object(TY_VARIABLE, "Number");
					r1->data_number = rand();

					PUSH_STACK(r1);

					INCREF(r1);
				} else if(strcmp(name->data_string, "delay") == 0) {
					Object *sec = POP_STACK();

					sleep((int)sec->data_number);

					PUSH_STACK(empty_return);

					DECREF(sec);
					INCREF(empty_return);
				} else if(strcmp(name->data_string, "string_length") == 0) {
					Object *i = POP_STACK();

					int length = strlen(i->data_string);

					Object *r = new_object(TY_VARIABLE, "Number");
					r->data_number = length;

					PUSH_STACK(r);

					DECREF(i);
					INCREF(r);
				} else if(strcmp(name->data_string, "string_at") == 0) {
					Object *i = POP_STACK();
					Object *index = POP_STACK();

					Object *r = new_object(TY_VARIABLE, "Number");
					r->data_number = (int)i->data_string[(int)index->data_number];

					PUSH_STACK(r);

					DECREF(i);
					DECREF(index);
					INCREF(r);
				} else {
					printf("VM:: unknown syscall %s\n", name->data_string);
					exit(1);
				}

				DECREF(name);
			}
			break;
			case OP_NEW: {
				List args;
				list_clear(&args);

				for(int i = 0; i < current->left; i++) {
					Object *arg = POP_STACK();
					list_insert(list_end(&args), arg);
				}

				Object *name = POP_STACK();

				Class *class = get_class(name->data_string);
				if(!class) {
					printf("VM:: cannot clone class %s\n", name->data_string);
					exit(1);
				}

				Object *instance = new_object(TY_VARIABLE, name->data_string);

				INCREF(instance);
				
				Method *constructor = get_method(name->data_string, "constructor");
				if(constructor) {
					DECREF(eval(instance, constructor, &args));
				}
				
				PUSH_STACK(instance);

				DECREF(name);
			}
			break;
			case OP_NEWARRAY: {
				Object *size = POP_STACK();
				Object *name = POP_STACK();

				Object *instance = new_object(TY_ARRAY, name->data_string);
				instance->array = malloc(sizeof(Object *) * (int)size->data_number);

				for(int i = 0; i < (int)size->data_number; i++) {
					instance->array[i] = empty_return;
					INCREF(empty_return);
				}

				PUSH_STACK(instance);

				DECREF(size);
				DECREF(name);
				INCREF(instance);
			}
			break;
			case OP_LOAD_ARRAY: {
				Object *index = POP_STACK();
				Object *instance = POP_STACK();

				Object *item = instance->array[(int)index->data_number];

				PUSH_STACK(item);

				DECREF(index);
				DECREF(instance);
				INCREF(item);
			}
			break;
			case OP_STORE_ARRAY: {
				Object *index = POP_STACK();
				Object *instance = POP_STACK();
				Object *value = POP_STACK();

				DECREF(instance->array[(int)index->data_number]);
				instance->array[(int)index->data_number] = value;
				INCREF(value);

				DECREF(index);
				DECREF(instance);
				DECREF(value);
			}
			break;
			case OP_JMPIFT: {
				Object *v1 = POP_STACK();
				Object *v2 = POP_STACK();

				if(v2->data_number == v1->data_number) {
					Op *jmp_to = op_at(&method->op, current->left);
					if(!jmp_to) {
						printf("VM:: Unable to jump to op %i\n", (int)current->left);
						exit(1);
					}

					current = jmp_to;
					
					DECREF(v1);
					DECREF(v2);

					continue;
				}

				DECREF(v1);
				DECREF(v2);
			}
			break;
			case OP_JMP: {
				Op *jmp_to = op_at(&method->op, current->left);
				if(!jmp_to) {
					printf("VM:: Unable to jump to op %i\n", (int)current->left);
					exit(1);
				}

				current = jmp_to;
				continue;
			}
			break;
			case OP_RET: {
				DECREF(ret);
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

		current = (Op*)list_next((ListNode*)current);
	}

	cleanup:;

	// remove vars
	while(!list_empty(&varlist)) {
		Var *var = (Var*)list_remove(list_begin(&varlist));
		
		free_var(var);
	}

	return ret;
}

void intepreter(const char *input) {
	/* code */
	signal(SIGPIPE, SIG_IGN);

	list_clear(&globals);

	list_clear(&program);

	load_file(input);

	emit_print();

	for(ListNode *cn = list_begin(&program); cn != list_end(&program); cn = list_next(cn)) {
		Class *c = (Class*)cn;

		Object *var = new_object(TY_VARIABLE, c->name);
		store_var(&globals, c->name, var);
	}

	empty_return = new_object(TY_VARIABLE, "Number");
	empty_return->data_number = 0;
	INCREF(empty_return);

	Method *method_main = get_method("Main", "main");
	if(!method_main) {
		printf("entry point method main not found\n");
		exit(1);
	}

	eval(NULL, method_main, NULL);
}