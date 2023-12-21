#include <stdio.h>
#include "chip.h"
#include "list.h"
#include "parse.h"
#include "codegen.h"
#include "semantic.h"

int main(int argc, char const *argv[]) {
	/* code */
	if(argc > 0 && argv[1] != NULL) {
		char *input = read_file((char *)argv[1]);

		List tokens;
		tokenize(input, &tokens);
		
		Node *nodes = parse(&tokens);

		semantic(nodes);

		gen(nodes);

		intepreter("a.out");
	} else {
		printf("usage: %s <file>\n", argv[0]);
	}

	return 0;
}