#ifndef __HIR_LEXER_C__
#define __HIR_LEXER_C__

#include "lexer.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

void hir_initLexer(hir_Lexer *lexer, const char *source)
{
	lexer->start = source;
	lexer->current = source;
	lexer->line = 1;
}

static hir_Token hir_makeToken(hir_Lexer *lexer, hir_TokenType type)
{
	hir_Token token;
	token.type = type;
	token.start = lexer->start;
	token.length = (int)(lexer->current - lexer->start);
	token.line = lexer->line;
	return token;
}

static hir_Token hir_errorToken(hir_Lexer *lexer, const char *message)
{
	hir_Token token;
	token.type = HIR_TOKEN_ERROR;
	token.start = message;
	token.length = (int)strlen(message);
	token.line = lexer->line;
	return token;
}

static bool hir_isAtEnd(hir_Lexer *lexer)
{
	return *lexer->current == '\0';
}

static inline void hir_skip(hir_Lexer *lexer)
{
	lexer->current++;
}

static inline char hir_advance(hir_Lexer *lexer)
{
	lexer->current++;
	return lexer->current[-1];
}

static inline char hir_peek(hir_Lexer *lexer)
{
	return *lexer->current;
}

static inline char hir_peekNext(hir_Lexer *lexer)
{
	return hir_isAtEnd(lexer) ? '\0' : lexer->current[1];
}

static inline bool isAlpha(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static inline bool isDigit(char c)
{
	return c >= '0' && c <= '9';
}

static inline bool isValidID(char c)
{
	return isAlpha(c) || isDigit(c) || c == '$' || c == '-';
}

static void hir_skipWhitespace(hir_Lexer *lexer)
{
	for (;;) {
		char c = hir_peek(lexer);
		switch (c) {
			case ' ':
			case '\r':
			case '\t':
				hir_skip(lexer);
				break;
			case '\n':
				lexer->line++;
				hir_skip(lexer);
				break;
			case '#':
				while (hir_peek(lexer) != '\n' && !hir_isAtEnd(lexer)) {
					hir_skip(lexer);
				}
				break;
			default:
				return;
		}
	}
}

static hir_Token hir_string(hir_Lexer *lexer)
{
	lexer->start++; /* skip first `"` "*/
	while (hir_peek(lexer) != '"' && !hir_isAtEnd(lexer)) {
		if (hir_peek(lexer) == '\n') {
			lexer->line++;
		}
		hir_skip(lexer);
	}

	if (hir_isAtEnd(lexer)) {
		return hir_errorToken(lexer, "unterminated string");
	}

	hir_Token token = hir_makeToken(lexer, HIR_TOKEN_STRING);
	hir_skip(lexer); /* Consume the closing `"` */
	return token;
}

static hir_Token hir_number(hir_Lexer *lexer)
{
	while (isDigit(hir_peek(lexer))) {
		hir_skip(lexer);
	}

	/* Look for a decimal */
	if (hir_peek(lexer) == '.' && isDigit(hir_peekNext(lexer))) {
		hir_skip(lexer); /* Consume the `.` */

		while (isDigit(hir_peek(lexer))) {
			hir_skip(lexer);
		}
	}

	return hir_makeToken(lexer, HIR_TOKEN_NUMBER);
}

static hir_Token hir_identifier(hir_Lexer *lexer)
{
	/* skip the `$` */
	lexer->start++;

	while (isValidID(hir_peek(lexer))) {
		hir_skip(lexer);
	}

	return hir_makeToken(lexer, HIR_TOKEN_ID);
}

static hir_TokenType hir_checkKeyword(hir_Lexer *lexer, int start, int length, const char *rest, hir_TokenType type)
{
	if (lexer->current - lexer->start == start + length && memcmp(lexer->start + start, rest, length) == 0) {
		return type;
	}
	return HIR_TOKEN_ID;
}

/* Check for operators using a trie.
 * Never-nesters fear this function */
static hir_TokenType hir_operatorType(hir_Lexer *lexer)
{
	switch (lexer->start[0]) {
		case 'a': {
			if (lexer->current - lexer->start > 1) {
				switch (lexer->start[1]) {
					case 'd': return hir_checkKeyword(lexer, 2, 1, "d", HIR_TOKEN_ADD);
					case 'n': return hir_checkKeyword(lexer, 2, 1, "d", HIR_TOKEN_AND);
				}
			}
			break;
		}
		case 'c': return hir_checkKeyword(lexer, 1, 5, "oncat", HIR_TOKEN_CONCAT);
		case 'd': {
			if (lexer->current - lexer->start > 1) {
				switch (lexer->start[1]) {
					case 'i': return hir_checkKeyword(lexer, 2, 1, "v", HIR_TOKEN_DIV);
					case 'e': return hir_checkKeyword(lexer, 2, 7, "fglobal", HIR_TOKEN_DEFGLOBAL);
				}
			}
			break;
		}
		case 'e': {
			if (lexer->current - lexer->start > 1) {
				switch (lexer->start[1]) {
					case 'x': return hir_checkKeyword(lexer, 2, 2, "it", HIR_TOKEN_EXIT);
					case 'q': return HIR_TOKEN_EQ;
				}
			}
			break;
		}
		case 'f': return hir_checkKeyword(lexer, 1, 4, "alse", HIR_TOKEN_FALSE);
		case 'g': {
			if (lexer->current - lexer->start > 1) {
				switch (lexer->start[1]) {
					case 'e': return hir_checkKeyword(lexer, 2, 7, "tglobal", HIR_TOKEN_GETGLOBAL);
					case 't': {
						if (hir_checkKeyword(lexer, 2, 2, "eq", HIR_TOKEN_GTEQ)) {
							return HIR_TOKEN_GTEQ;
						}
						return hir_checkKeyword(lexer, 1, 1, "t", HIR_TOKEN_GT);
					}
				}
			}
			break;
		}
		case 'l': {
			if (lexer->current - lexer->start > 1) {
				if (hir_checkKeyword(lexer, 1, 3, "teq", HIR_TOKEN_LTEQ)) {
					return HIR_TOKEN_LTEQ;
				}
				return hir_checkKeyword(lexer, 1, 1, "t", HIR_TOKEN_LT);
			}
			break;
		}
		case 'm': return hir_checkKeyword(lexer, 1, 2, "ul", HIR_TOKEN_MUL);
		case 'n': {
			if (lexer->current - lexer->start > 1) {
				switch (lexer->start[1]) {
					case 'e': {
						if (lexer->current - lexer->start > 1) {
							switch (lexer->start[2]) {
								case 'g': return hir_checkKeyword(lexer, 2, 4, "gate", HIR_TOKEN_NEGATE);
								case 'q': return HIR_TOKEN_NEQ;
							}
						}
						break;
					}
					case 'i': return hir_checkKeyword(lexer, 2, 1, "l", HIR_TOKEN_NIL);
					case 'o': return hir_checkKeyword(lexer, 2, 1, "t", HIR_TOKEN_NOT);
				}
			}
			break;
		}
		case 'o': return hir_checkKeyword(lexer, 1, 1, "r", HIR_TOKEN_OR);
		case 'p': {
			if (lexer->current - lexer->start > 1) {
				switch (lexer->start[1]) {
					case 'r': return hir_checkKeyword(lexer, 2, 3, "int", HIR_TOKEN_PRINT);
					case 'u': return hir_checkKeyword(lexer, 2, 2, "sh", HIR_TOKEN_PUSH);
					case 'o': return hir_checkKeyword(lexer, 2, 1, "p", HIR_TOKEN_POP);
				}
			}
			break;
		}
		case 'r': return hir_checkKeyword(lexer, 1, 5, "eturn", HIR_TOKEN_RETURN);
		case 's': return hir_checkKeyword(lexer, 1, 2, "ub", HIR_TOKEN_SUB);
		case 't': return hir_checkKeyword(lexer, 1, 3, "rue", HIR_TOKEN_TRUE);
		case 'x': return hir_checkKeyword(lexer, 1, 2, "or", HIR_TOKEN_XOR);
	}
	return HIR_TOKEN_ERROR;
}

static hir_Token hir_operator(hir_Lexer *lexer)
{
	while (isAlpha(hir_peek(lexer)) || isDigit(hir_peek(lexer))) {
		hir_skip(lexer);
	}
	hir_TokenType type = hir_operatorType(lexer);
	return type == HIR_TOKEN_ERROR ?
		hir_errorToken(lexer, "invalid operator") :
		hir_makeToken(lexer, type);
}

hir_Token hir_scanToken(hir_Lexer *lexer)
{
	hir_skipWhitespace(lexer);
	lexer->start = lexer->current;

	if (hir_isAtEnd(lexer)) {
		return hir_makeToken(lexer, HIR_TOKEN_EOF);
	}

	char c = hir_advance(lexer);

	if (isAlpha(c)) {
		return hir_operator(lexer);
	} else if (isDigit(c)) {
		return hir_number(lexer);
	} else {
		switch (c) {
			case '.': return hir_makeToken(lexer, HIR_TOKEN_DOT);
			case '=': return hir_makeToken(lexer, HIR_TOKEN_EQUALS);
			case '$': return hir_identifier(lexer);
			case '"': return hir_string(lexer);
		}
	}

	return hir_errorToken(lexer, "unexpected character");
}

void hir_printToken(hir_Token *token)
{
	switch (token->type) {
		/* Single characters */
		case HIR_TOKEN_DOT: fputs("DOT", stdout); break;
		case HIR_TOKEN_EQUALS: fputs("EQUALS", stdout); break;
		case HIR_TOKEN_DOLLAR: fputs("DOLLAR", stdout); break;
		/* Literals */
		case HIR_TOKEN_ID: fputs("ID", stdout); break;
		case HIR_TOKEN_TRUE: fputs("TRUE", stdout); break;
		case HIR_TOKEN_FALSE: fputs("FALSE", stdout); break;
		case HIR_TOKEN_NIL: fputs("NIL", stdout); break;
		case HIR_TOKEN_NUMBER: fputs("NUMBER", stdout); break;
		case HIR_TOKEN_STRING: fputs("STRING", stdout); break;
		/* Operations */
                case HIR_TOKEN_PUSH: fputs("PUSH", stdout); break;
                case HIR_TOKEN_POP: fputs("POP", stdout); break;
                case HIR_TOKEN_ADD: fputs("ADD", stdout); break;
                case HIR_TOKEN_SUB: fputs("SUB", stdout); break;
                case HIR_TOKEN_MUL: fputs("MUL", stdout); break;
                case HIR_TOKEN_DIV: fputs("DIV", stdout); break;
                case HIR_TOKEN_NEGATE: fputs("NEGATE", stdout); break;
                case HIR_TOKEN_DEFGLOBAL: fputs("DEFGLOBAL", stdout); break;
                case HIR_TOKEN_GETGLOBAL: fputs("GETGLOBAL", stdout); break;
                case HIR_TOKEN_NOT: fputs("NOT", stdout); break;
		case HIR_TOKEN_AND: fputs("AND", stdout); break;
		case HIR_TOKEN_OR: fputs("OR", stdout); break;
		case HIR_TOKEN_XOR: fputs("XOR", stdout); break;
		case HIR_TOKEN_EQ: fputs("EQ", stdout); break;
		case HIR_TOKEN_NEQ: fputs("NEQ", stdout); break;
		case HIR_TOKEN_GT: fputs("GT", stdout); break;
		case HIR_TOKEN_LT: fputs("LT", stdout); break;
		case HIR_TOKEN_GTEQ: fputs("GTEQ", stdout); break;
		case HIR_TOKEN_LTEQ: fputs("LTEQ", stdout); break;
		case HIR_TOKEN_CONCAT: fputs("CONCAT", stdout); break;
                case HIR_TOKEN_PRINT: fputs("PRINT", stdout); break;
                case HIR_TOKEN_RETURN: fputs("RETURN", stdout); break;
                case HIR_TOKEN_EXIT: fputs("EXIT", stdout); break;
		// Misc
		case HIR_TOKEN_ERROR: fputs("ERROR", stdout); break;
		case HIR_TOKEN_EOF: fputs("EOF", stdout); break;
	}
        printf(" '%.*s'\n", token->length, token->start);
}

void hir_lex(hir_Lexer *lexer)
{
	int line = -1;
	for (;;) {
		hir_Token token = hir_scanToken(lexer);
		if (token.line != line) {
			printf("%4d ", token.line);
			line = token.line;
		} else {
			fputs("   | ", stdout);
		}
		hir_printToken(&token);

		if (token.type == HIR_TOKEN_EOF) {
			break;
		}
	}
}

#endif
