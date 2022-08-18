#include <stdlib.h>
#include <stdio.h>
#include "eval.h"

static Token *new_token(TokenType type, char *data, int length) {
	Token *token  = malloc(sizeof(Token));
	token->data   = data;
	token->length = length;
	token->type   = type;

	return token;
}

static bool is_identifier(char bit) {
	return ('a' <= bit && bit <= 'z') || ('A' <= bit && bit <= 'Z') || bit == '_';
}

static bool is_numeric(char bit) {
	return (bit >= '0' && bit <= '9');
}

static bool is_number(char bit) {
	return is_numeric(bit) || bit == '.';
}

static bool is_space(char bit) {
	return (bit == '	' || bit == ' ' || bit == 0x0d || bit == 0x0a);
}

static bool is_punctuation(char bit) {
	return (
		bit == '+' || 
		bit == '-' || 
		bit == '*' || 
		bit == '/' ||
		bit == '(' ||
		bit == ')'
	);
}

void tokenize(char *input, List *tokens) {
	list_clear(tokens);

	while(*input != '\0') {
		if(is_identifier(*input)) {
			char *start = input;
			while(is_identifier(*input) || is_numeric(*input)) {
				input++;
			}

			Token *token = new_token(TK_IDENTIFIER, start, input - start);
			list_insert(list_end(tokens), token);
			continue;
		} else if(is_number(*input)) {
			char *start = input;
			while(is_number(*input)) {
				input++;
			}

			Token *token = new_token(TK_NUMBER, start, input - start);
			list_insert(list_end(tokens), token);
			continue;
		} else if(is_space(*input)) {
			input++;
			continue;
		} else if(is_punctuation(*input)) {
			Token *token = new_token(TK_PUNCTUATION, input, 1);
			list_insert(list_end(tokens), token);
			input++;
			continue;
		} else {
			printf("unknown token %c\n", *input);
			exit(1);
		}

	}

	Token *token = new_token(TK_EOF, NULL, 0);
	list_insert(list_end(tokens), token);
}