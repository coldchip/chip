#include <stdio.h>
#include "eval.h"

int main(int argc, char const *argv[]) {
	/* code */
	if(argc > 0 && argv[1] != NULL) {
		char *input = read_file((char *)argv[1]);

		List tokens;
		tokenize(input, &tokens);
		
		Node *nodes = parse(&tokens);

		List program;
		gen(nodes, &program);
		run(&program);
	} else {
		printf("usage: eval <file>\n");
	}

	return 0;
}