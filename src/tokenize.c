#include <stdlib.h>
#include <stdio.h>
#include "eval.h"

static Token *new_token(TokenType type, char *data, int length, Token *prev) {
	Token *token  = malloc(sizeof(Token));
	token->data   = data;
	token->length = length;
	token->type   = type;
	token->next   = NULL;

	if(prev) {
		prev->next = token;
	}

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

Token *tokenize(char *input) {
	Token head = {};
	Token *token = &head;

	while(*input != '\0') {
		if(is_identifier(*input)) {
			char *start = input;
			while(is_identifier(*input) || is_numeric(*input)) {
				input++;
			}

			token = new_token(TK_IDENTIFIER, start, input - start, token);
			continue;
		} else if(is_number(*input)) {
			char *start = input;
			while(is_number(*input)) {
				input++;
			}

			token = new_token(TK_NUMBER, start, input - start, token);
			continue;
		} else if(is_space(*input)) {
			input++;
			continue;
		} else if(is_punctuation(*input)) {
			token = new_token(TK_PUNCTUATION, input, 1, token);
			input++;
			continue;
		} else {
			printf("unknown token %c\n", *input);
			exit(1);
		}

	}

	token = new_token(TK_EOF, NULL, 0, token);

	return head.next;
}