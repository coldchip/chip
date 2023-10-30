#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include "chip.h"

static List constants;
static List program;

static void load_file(const char *name) {
	FILE *fp = fopen(name, "rb");
	if(!fp) {
		printf("unable to load file %s\n", name);
		exit(1);
	}

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

		Constant *constant = malloc(sizeof(Constant));
		constant->data = strdup(constant_data);

		list_insert(list_end(&constants), constant);
	}

	fseek(fp, 4, SEEK_SET);

	int class_count = 0;
	fread(&class_count, sizeof(class_count), 1, fp);

	for(int i = 0; i < class_count; i++) {
		short method_count = 0;
		short class_name = 0;
		fread(&method_count, sizeof(method_count), 1, fp);
		fread(&class_name, sizeof(class_name), 1, fp);

		Class *class = malloc(sizeof(Class));
		class->name = lookup_constant(class_name);
		list_clear(&class->method);

		for(int x = 0; x < method_count; x++) {
			short op_count = 0;
			short method_name = 0;
			fread(&op_count, sizeof(op_count), 1, fp);
			fread(&method_name, sizeof(method_name), 1, fp);

			Method *method = malloc(sizeof(Method));
			method->name = lookup_constant(method_name);
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

static void emit_print() {
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
						printf("\t%i\tLOAD_VAR\t%s\n", line, lookup_constant(ins->left));
					}
					break;
					case OP_STORE_VAR: {
						printf("\t%i\tSTORE_VAR\t%s\n", line, lookup_constant(ins->left));
					}
					break;
					case OP_POP: {
						printf("\t%i\tPOP\t\n", line);
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
						printf("\t%i\tLOAD_CONST\t%s\n", line, lookup_constant(ins->left));
					}
					break;
					case OP_LOAD_MEMBER: {
						printf("\t%i\tLOAD_MEMBER\t%s\n", line, lookup_constant(ins->left));
					}
					break;
					case OP_STORE_MEMBER: {
						printf("\t%i\tSTORE_MEMBER\t%s\n", line, lookup_constant(ins->left));
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

static char *lookup_constant(int pos) {
	int i = 0;

	for(ListNode *position = list_begin(&constants); position != list_end(&constants); position = list_next(position)) {
		Constant *constant = (Constant*)position;
		if(i == pos) {
			return constant->data;
		}
		i++;
	}

	return NULL;
}

static Op *op_at(List *program, int line) {
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

static void store_var(List *vars, char *name, Object *object) {
	if(load_var(vars, name)) {
		for(ListNode *i = list_begin(vars); i != list_end(vars); i = list_next(i)) {
			Var *var = (Var*)i;
			if(strcmp(name, var->name) == 0) {
				var->object = object;
			}
		}
	} else {
		Var *var = malloc(sizeof(Var));
		var->name = strdup(name);
		var->object = object;
		list_insert(list_end(vars), var);
	}
}

static Object *load_var(List *vars, char *name) {
	for(ListNode *i = list_begin(vars); i != list_end(vars); i = list_next(i)) {
		Var *var = (Var*)i;
		if(strcmp(name, var->name) == 0) {
			return var->object;
		}
	}
	return NULL;
}

static Class *get_class(char *name) {
	for(ListNode *i = list_begin(&program); i != list_end(&program); i = list_next(i)) {
		Class *class = (Class*)i;
		if(strcmp(name, class->name) == 0) {
			return class;
		}
	}
	return NULL;
}

static Method *get_method(char *name1, char *name2) {
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

int count = 0;

Object *new_object(Type type, char *name) {

	count++;

	Object *o = malloc(sizeof(Object));
	o->type = type;
	o->name = name;

	list_clear(&o->varlist);

	if(type == TY_CUSTOM) {
		Class *skeleton = get_class(name);
		if(skeleton) {
			for(ListNode *i = list_begin(&skeleton->method); i != list_end(&skeleton->method); i = list_next(i)) {
				Method *method = (Method*)i;
				Object *var = new_object(TY_FUNCTION, method->name);
				var->bound = o;
				var->method = method;

				store_var(&o->varlist, method->name, var);
			}
		} else {
			printf("Error, class %s not defined\n", name);
			exit(1);
		}
	}

	if(type == TY_STRING) {
		Object *l = new_object(TY_NUMBER, "Number");
		l->data_number = 666;

		Var *var = malloc(sizeof(Var));
		var->name = strdup("length");
		var->object = l;

		list_insert(list_end(&o->varlist), var);
	}

	return o;
}

Object *eval(Object *instance, Method *method, List *inject) {
	/*
		assume if instance == NULL then method is static
	*/
	List varlist;
	list_clear(&varlist);

	if(instance != NULL) {
		store_var(&varlist, "this", instance);
	}
	
	Op *current = (Op*)list_begin(&method->op);

	Object *stack[8192];
	int sp = 0;

	Object *ret = new_object(TY_NUMBER, "Number");
	ret->data_number = 0;

	if(inject) {
		for(ListNode *i = list_begin(inject); i != list_end(inject); i = list_next(i)) {
			Object *param = (Object*)i;
			PUSH_STACK(param);
		}
	}

	while(current != (Op*)list_end(&method->op)) {
		switch(current->op) {
			case OP_LOAD_VAR: {
				Object *o = load_var(&varlist, lookup_constant(current->left));
				if(o) {
					PUSH_STACK(o);
				} else {
					printf("VM:: Unable to load variable %s as it is not found\n", lookup_constant(current->left));
					exit(1);
				}
			}
			break;
			case OP_STORE_VAR: {
				Object *v1 = POP_STACK();
				store_var(&varlist, lookup_constant(current->left), v1);

				printf("sp: %i\n", sp);
			}
			break;
			case OP_LOAD_NUMBER: {
				Object *o = new_object(TY_NUMBER, "Number");
				o->data_number = current->left;
				PUSH_STACK(o);
			}
			break;
			case OP_LOAD_CONST: {
				Object *o = new_object(TY_STRING, "String");
				o->data_string = lookup_constant(current->left);
				PUSH_STACK(o);
			}
			break;
			case OP_LOAD_MEMBER: {
				Object *instance = POP_STACK();

				Object *v2 = load_var(&instance->varlist, lookup_constant(current->left));
				if(v2) {
					PUSH_STACK(v2);
				} else {
					printf("Unknown variable member %s\n", lookup_constant(current->left));
					exit(1);
				}
			}
			break;
			case OP_STORE_MEMBER: {
				Object *v1 = POP_STACK();
				Object *v2 = POP_STACK();

				store_var(&v1->varlist, lookup_constant(current->left), v2);
			}
			break;
			case OP_POP: {
				POP_STACK();
			}
			break;
			case OP_CMPGT: {
				Object *v1 = POP_STACK();
				Object *v2 = POP_STACK();

				Object *v3 = new_object(TY_NUMBER, "Number");
				v3->data_number = v2->data_number > v1->data_number;

				PUSH_STACK(v3);
			}
			break;
			case OP_CMPLT: {
				Object *v1 = POP_STACK();
				Object *v2 = POP_STACK();

				Object *v3 = new_object(TY_NUMBER, "Number");
				v3->data_number = v2->data_number < v1->data_number;

				PUSH_STACK(v3);
			}
			break;
			case OP_ADD: {
				Object *v1 = POP_STACK();
				Object *v2 = POP_STACK();

				if(v1->type == TY_NUMBER && v2->type == TY_NUMBER) {
					Object *v3 = new_object(TY_NUMBER, "Number");
					v3->data_number = v2->data_number + v1->data_number;
					
					PUSH_STACK(v3);
				} else if(v1->type == TY_NUMBER && v2->type == TY_STRING) {
					Object *v3 = new_object(TY_STRING, "String");

					char str[512];
					sprintf(str, "%f", v1->data_number);

					char *s = malloc(strlen(str) + strlen(v2->data_string) + 1);
					strcpy(s, v2->data_string);
					strcat(s, str);
					v3->data_string = s;
					
					PUSH_STACK(v3);
				} else if(v1->type == TY_STRING && v2->type == TY_NUMBER) {
					Object *v3 = new_object(TY_STRING, "String");

					char str[512];
					sprintf(str, "%f", v2->data_number);

					char *s = malloc(strlen(v1->data_string) + strlen(str) + 1);
					strcpy(s, str);
					strcat(s, v1->data_string);
					v3->data_string = s;
					
					PUSH_STACK(v3);
				} else if(v1->type == TY_STRING && v2->type == TY_STRING) {
					Object *v3 = new_object(TY_STRING, "String");

					char *s = malloc(strlen(v1->data_string) + strlen(v2->data_string) + 1);
					strcpy(s, v2->data_string);
					strcat(s, v1->data_string);
					v3->data_string = s;
					
					PUSH_STACK(v3);
				} else {
					printf("Unable to binary add unknown types\n");
					exit(1);
				}
			}
			break;
			case OP_SUB: {
				Object *v1 = POP_STACK();
				Object *v2 = POP_STACK();

				Object *v3 = new_object(TY_NUMBER, "Number");
				v3->data_number = v2->data_number - v1->data_number;
				
				PUSH_STACK(v3);
			}
			break;
			case OP_MUL: {
				Object *v1 = POP_STACK();
				Object *v2 = POP_STACK();

				Object *v3 = new_object(TY_NUMBER, "Number");
				v3->data_number = v2->data_number * v1->data_number;
				
				PUSH_STACK(v3);
			}
			break;
			case OP_DIV: {
				Object *v1 = POP_STACK();
				Object *v2 = POP_STACK();

				Object *v3 = new_object(TY_NUMBER, "Number");
				v3->data_number = v2->data_number / v1->data_number;
				
				PUSH_STACK(v3);
			}
			break;
			case OP_MOD: {
				Object *v1 = POP_STACK();
				Object *v2 = POP_STACK();

				Object *v3 = new_object(TY_NUMBER, "Number");
				v3->data_number = ((int)v2->data_number) % ((int)v1->data_number);
				
				PUSH_STACK(v3);
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

				if(instance->type == TY_FUNCTION) {
					Object *ret = eval(instance->bound, instance->method, &args);
					
					PUSH_STACK(ret);
				} else {
					printf("VM:: unknown function call %s\n", instance->data_string);
					exit(1);
				}
			}
			break;
			case OP_SYSCALL: {
				Object *name = POP_STACK();

				List args;
				list_clear(&args);

				if(strcmp(name->data_string, "print") == 0) {
					Object *arg = POP_STACK();
					if(arg->type == TY_STRING) {
						printf("%s", arg->data_string);
					} else if(arg->type == TY_NUMBER) {
						printf("%f", arg->data_number);
					} else {
						printf("VM:: print of unknown type\n");
						exit(1);
					}

					PUSH_STACK(ret);
				} else if(strcmp(name->data_string, "socket") == 0) {
					int sockfd = socket(AF_INET, SOCK_STREAM, 0);

					setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

					Object *r = new_object(TY_NUMBER, "Number");
					r->data_number = sockfd;

					PUSH_STACK(r);
				} else if(strcmp(name->data_string, "bind") == 0) {
					Object *fd   = POP_STACK();
					Object *ip   = POP_STACK();
					Object *port = POP_STACK();

					struct sockaddr_in servaddr;
					servaddr.sin_family = AF_INET;
					servaddr.sin_addr.s_addr = inet_addr(ip->data_string);
					servaddr.sin_port = htons((int)port->data_number);

					bind((int)fd->data_number, (struct sockaddr*)&servaddr, sizeof(servaddr));
					listen((int)fd->data_number, 5);

					PUSH_STACK(ret);
				} else if(strcmp(name->data_string, "accept") == 0) {
					Object *fd = POP_STACK();

					int newfd = accept((int)fd->data_number, NULL, 0);

					Object *r = new_object(TY_NUMBER, "Number");
					r->data_number = newfd;

					PUSH_STACK(r);
				} else if(strcmp(name->data_string, "read") == 0) {
					Object *fd = POP_STACK();

					char data[8192];
					read((int)fd->data_number, data, sizeof(data));

					Object *o = new_object(TY_STRING, "String");
					o->data_string = strdup(data);

					PUSH_STACK(o);
				} else if(strcmp(name->data_string, "write") == 0) {
					Object *fd = POP_STACK();
					Object *data = POP_STACK();

					int w = write((int)fd->data_number, data->data_string, strlen(data->data_string));
					Object *r1 = new_object(TY_NUMBER, "Number");
					r1->data_number = w;

					PUSH_STACK(r1);
				} else if(strcmp(name->data_string, "close") == 0) {
					Object *fd = POP_STACK();

					close((int)fd->data_number);

					PUSH_STACK(ret);
				} else {
					printf("VM:: unknown syscall %s\n", name->data_string);
					exit(1);
				}
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
				if(class) {
					Object *instance = new_object(TY_CUSTOM, strdup(name->data_string));
					
					Method *constructor = get_method(strdup(name->data_string), "constructor");
					if(constructor) {
						eval(instance, constructor, &args);
					}
					
					PUSH_STACK(instance);
				} else {
					printf("VM:: cannot clone class %s\n", name->data_string);
					exit(1);
				}
			}
			break;
			case OP_JMPIFT: {
				Object *v1 = POP_STACK();
				Object *v2 = POP_STACK();

				if(v2->data_number == v1->data_number) {
					Op *jmp_to = op_at(&method->op, current->left);
					if(jmp_to) {
						current = jmp_to;
						continue;
					} else {
						printf("VM:: Unable to jump to op %i\n", (int)current->left);
						exit(1);
					}
				}
			}
			break;
			case OP_JMP: {
				Op *jmp_to = op_at(&method->op, current->left);
				if(jmp_to) {
					current = jmp_to;
					continue;
				} else {
					printf("VM:: Unable to jump to op %i\n", (int)current->left);
					exit(1);
				}
			}
			break;
			case OP_RET: {
				ret = POP_STACK();
			}
			break;
		}

		current = (Op*)list_next((ListNode*)current);
	}

	// printf("\n\nVARLIST\n-----------------\n");

	// for(ListNode *i = list_begin(&varlist); i != list_end(&varlist); i = list_next(i)) {
	// 	Var *var = (Var*)i;
	// 	switch(var->object->type) {
	// 		case TY_NUMBER: {
	// 			printf("%s\tNumber@%p\t%f\n", var->name, var->object, var->object->data_number);
	// 		}
	// 		break;
	// 		case TY_STRING: {
	// 			printf("%s\tString@%p\t%s\n", var->name, var->object, var->object->data_string);
	// 		}
	// 		break;
	// 		case TY_FUNCTION: {
	// 			printf("%s\t#Function@%p\t\n", var->name, var->object);
	// 		}
	// 		break;
	// 		case TY_CUSTOM: {
	// 			printf("%s\t%s@%p\t\n", var->name, var->object->name, var->object);
	// 		}
	// 		break;
	// 	}
	// }

	// printf("\n\n-----------------\n");
	// printf("sp: %i\n", sp);

	return ret;
}

void intepreter(const char *input) {
	/* code */
	signal(SIGPIPE, SIG_IGN);

	list_clear(&constants);
	list_clear(&program);

	load_file(input);

	emit_print();

	Method *method_main = get_method("Main", "main");
	if(!method_main) {
		printf("entry point method main not found\n");
		exit(1);
	}

	eval(NULL, method_main, NULL);
}