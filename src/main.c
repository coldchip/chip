#include <stdio.h>
#include <string.h>
#include "chip.h"
#include "list.h"
#include "parse.h"
#include "codegen.h"
#include "semantic.h"
#include "intepreter.h"

int main(int argc, char const *argv[]) {
	/* code */
	if(argc > 0 && argv[1] != NULL && argv[2] != NULL) {

		if(strcmp(argv[1], "compile") == 0) {
			char *input = read_file((char *)argv[2]);

			List tokens;
			list_clear(&tokens);
			tokenize(input, &tokens);
			
			Node *nodes = parse(&tokens);

			semantic(nodes);

			gen(nodes, "a.out");
		} else if(strcmp(argv[1], "run") == 0) {
			intepreter(argv[2]);
		} else {
			printf("usage: %s compile|run <file>\n", argv[0]);
		}
	} else {
		printf("usage: %s compile|run <file>\n", argv[0]);
	}

	return 0;
}