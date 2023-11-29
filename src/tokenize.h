#ifndef TOKENIZE_H
#define TOKENIZE_H

#include <stdbool.h>
#include "list.h"

typedef enum {
	TK_IDENTIFIER,
	TK_NUMBER,
	TK_STRING,
	TK_PUNCTUATION,
	TK_EOF
} TokenType;

typedef struct _Token {
	ListNode node;
	TokenType type;
	char *data;
	int line;
} Token;

static Token      *new_token(TokenType type, char *data, int length, int line);

Token             *next(Token **token);
Token             *prev(Token **current);
bool               equals_string(Token **current, char *data);
bool               equals_type(Token **current, TokenType type);
bool               consume_string(Token **current, char *data);
bool               consume_type(Token **current, TokenType type);
void               expect_string(Token **current, char *data);
void               expect_type(Token **current, TokenType type);

static bool        is_identifier(char bit);
static bool        is_numeric(char bit);
static bool        is_number(char bit);
static bool        is_space(char bit);
static bool        is_break(char bit);
static bool        is_punctuation(char bit);
void               tokenize(char *input, List *tokens);

#endif