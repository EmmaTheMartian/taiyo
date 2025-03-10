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

static char hir_advance(hir_Lexer *lexer)
{
	lexer->current++;
	return lexer->current[-1];
}

static char hir_peek(hir_Lexer *lexer)
{
	return *lexer->current;
}

static char hir_peekNext(hir_Lexer *lexer)
{
	return hir_isAtEnd(lexer) ? '\0' : lexer->current[1];
}

static bool isAlpha(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool isDigit(char c)
{
	return c >= '0' && c <= '9';
}

static void hir_skipWhitespace(hir_Lexer *lexer)
{
	for (;;) {
		char c = hir_peek(lexer);
		switch (c) {
			case ' ':
			case '\r':
			case '\t':
				hir_advance(lexer);
				break;
			case '\n':
				lexer->line++;
				hir_advance(lexer);
				break;
			case '#':
				while (hir_peek(lexer) != '\n' && !hir_isAtEnd(lexer)) {
					hir_advance(lexer);
				}
				break;
			default:
				return;
		}
	}
}

static hir_Token hir_string(hir_Lexer *lexer)
{
	while (hir_peek(lexer) != '"' && !hir_isAtEnd(lexer)) {
		if (hir_peek(lexer) == '\n') {
			lexer->line++;
		}
		hir_advance(lexer);
	}

	if (hir_isAtEnd(lexer)) {
		return hir_errorToken(lexer, "unterminated string");
	}

	hir_advance(lexer); /* Consume the closing `"` */
	return hir_makeToken(lexer, HIR_TOKEN_STRING);
}

static hir_Token hir_number(hir_Lexer *lexer)
{
	while (isDigit(hir_peek(lexer))) {
		hir_advance(lexer);
	}

	/* Look for a decimal */
	if (hir_peek(lexer) == '.' && isDigit(hir_peekNext(lexer))) {
		hir_advance(lexer); /* Consume the `.` */

		while (isDigit(hir_peek(lexer))) {
			hir_advance(lexer);
		}
	}

	return hir_makeToken(lexer, HIR_TOKEN_NUMBER);
}

static hir_TokenType hir_checkKeyword(hir_Lexer *lexer, int start, int length, const char *rest, hir_TokenType type)
{
	if (lexer->current - lexer->start == start + length && memcmp(lexer->start + start, rest, length) == 0) {
		return type;
	}
	return HIR_TOKEN_ID;
}

static hir_TokenType hir_identifierType(hir_Lexer *lexer)
{
	/* Check for keywords */
	switch (lexer->start[0]) {
		case 'p': {
			switch (lexer->start[1]) {
				case 'u': return hir_checkKeyword(lexer, 2, 2, "sh", HIR_TOKEN_PUSH);
				case 'o': return hir_checkKeyword(lexer, 2, 1, "p", HIR_TOKEN_POP);
			}
		}
		case 'a': return hir_checkKeyword(lexer, 1, 2, "dd", HIR_TOKEN_ADD);
		case 's': return hir_checkKeyword(lexer, 1, 2, "ub", HIR_TOKEN_SUB);
		case 'm': return hir_checkKeyword(lexer, 1, 2, "ul", HIR_TOKEN_MUL);
		case 'd': return hir_checkKeyword(lexer, 1, 2, "iv", HIR_TOKEN_DIV);
		case 'n': return hir_checkKeyword(lexer, 1, 5, "egate", HIR_TOKEN_NEGATE);
		case 'r': return hir_checkKeyword(lexer, 1, 5, "eturn", HIR_TOKEN_RETURN);
		case 'e': return hir_checkKeyword(lexer, 1, 3, "xit", HIR_TOKEN_EXIT);
	}
	return HIR_TOKEN_ID;
}

static hir_Token hir_identifier(hir_Lexer *lexer)
{
	while (isAlpha(hir_peek(lexer)) || isDigit(hir_peek(lexer))) {
		hir_advance(lexer);
	}
	return hir_makeToken(lexer, hir_identifierType(lexer));
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
		return hir_identifier(lexer);
	} else if (isDigit(c)) {
		return hir_number(lexer);
	} else {
		switch (c) {
			case '.': return hir_makeToken(lexer, HIR_TOKEN_DOT);
			case '=': return hir_makeToken(lexer, HIR_TOKEN_EQUALS);
			case '$': return hir_makeToken(lexer, HIR_TOKEN_DOLLAR);
			case '"': return hir_string(lexer);
		}
	}

	return hir_errorToken(lexer, "unexpected character");
}

void hir_printToken(hir_Token *token)
{
	switch (token->type) {
		// Single-character tokens
		case HIR_TOKEN_DOT: fputs("DOT", stdout); break;
		case HIR_TOKEN_EQUALS: fputs("EQUALS", stdout); break;
		case HIR_TOKEN_DOLLAR: fputs("DOLLAR", stdout); break;
		// Literals
		case HIR_TOKEN_ID: fputs("ID", stdout); break;
		case HIR_TOKEN_NUMBER: fputs("NUMBER", stdout); break;
		case HIR_TOKEN_STRING: fputs("STRING", stdout); break;
		// Operations
                case HIR_TOKEN_PUSH: fputs("PUSH", stdout); break;
                case HIR_TOKEN_POP: fputs("POP", stdout); break;
                case HIR_TOKEN_ADD: fputs("ADD", stdout); break;
                case HIR_TOKEN_SUB: fputs("SUB", stdout); break;
                case HIR_TOKEN_MUL: fputs("MUL", stdout); break;
                case HIR_TOKEN_DIV: fputs("DIV", stdout); break;
                case HIR_TOKEN_NEGATE: fputs("NEGATE", stdout); break;
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
