#include <stdio.h>
#include "eval.h"

int main(int argc, char const *argv[]) {
	/* code */
	// ((8 * 2) / (2 * 2) * 9 / (2 * 5) + 560 / 90 / 2 * (100 + 4) -2 * (50 / 2)) * 9 / 8 / 7 / 6 / 5 / 4 / 3 * 1000 * (1 / 2) + (8 * 2 / 9 * 6 + 2 * 4 - 8 / 2 * 5)
	char input[] = "sin(3) * ((8 * 2) / (2 * 2) * 9 / (2 * 5) + 560 / 90 / 2 * (100 + 4) -2 * (50 / 2)) * 9 / 8 / 7 / 6 / 5 / 4 / 3 * 1000 * (1 / 2) + (8 * 2 / 9 * 6 + 2 * 4 - 8 / 2 * 5)";

	Token *tokens = tokenize(input);
	Node *nodes = parse(tokens);
	gen(nodes);

	// printf("\n");

	// while(tokens != NULL) {
	// 	printf("tok: %.*s\n", tokens->length, tokens->data);
	// 	tokens = tokens->next;
	// }

	return 0;
}