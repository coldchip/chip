#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "eval.h"

static Token *new_token(TokenType type, char *data, int length) {
	Token *token  = malloc(sizeof(Token));
	token->data   = data;
	token->length = length;
	token->type   = type;

	return token;
}

Token *next(Token **current) {
	*current = (Token*)list_next((ListNode*)*current);
	return *current;
}

Token *prev(Token **current) {
	*current = (Token*)list_previous((ListNode*)*current);
	return *current;
}

bool equals_string(Token **current, char *data) {
	return ((*current)->type != TK_EOF && memcmp((*current)->data, data, (*current)->length) == 0) && data[(*current)->length] == '\0';
}

bool equals_type(Token **current, TokenType type) {
	return ((*current)->type == type);
}

bool consume_string(Token **current, char *data) {
	if(equals_string(current, data)) {
		next(current);
		return true;
	}
	return false;
}

bool consume_type(Token **current, TokenType type) {
	if(equals_type(current, type)) {
		next(current);
		return true;
	}
	return false;
}

void expect_string(Token **current, char *data) {
	if(!consume_string(current, data)) {
		printf("expected token '%s'\n", data);
		exit(0);
	}
}

void expect_type(Token **current, TokenType type) {
	if(!consume_type(current, type)) {
		printf("expected type '%i'\n", type);
		exit(0);
	}
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
		bit == '%' ||
		bit == '=' || 
		bit == '(' ||
		bit == ')' ||
		bit == '{' ||
		bit == '}' ||
		bit == '>' ||
		bit == '<' ||
		bit == ',' ||
		bit == ';'
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
		} else if(*input == '\"') {
			input++;

			char *start = input;
			while(*input != '\"' && *input != '\0') {
				input++;
			}

			input++;

			Token *token = new_token(TK_STRING, start, (input - start) - 1);
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