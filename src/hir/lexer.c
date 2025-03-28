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

static hir_Token hir_label(hir_Lexer *lexer)
{
	/* skip the `:` */
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
 * Never-nesters fear this function.
 * TODO: There's gotta be a way to make this not so... insane */
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
		case 'b': {
			if (hir_checkKeyword(lexer, 1, 8, "ack_jump", HIR_TOKEN_BACK_JUMP) == HIR_TOKEN_BACK_JUMP) {
				return HIR_TOKEN_BACK_JUMP;
			} else if (hir_checkKeyword(lexer, 1, 11, "ack_jump_if", HIR_TOKEN_BACK_JUMP_IF) == HIR_TOKEN_BACK_JUMP_IF) {
				return HIR_TOKEN_BACK_JUMP_IF;
			}
			break;
		}
		case 'c': return hir_checkKeyword(lexer, 1, 5, "oncat", HIR_TOKEN_CONCAT);
		case 'd': {
			if (lexer->current - lexer->start > 1) {
				switch (lexer->start[1]) {
					case 'i': return hir_checkKeyword(lexer, 2, 1, "v", HIR_TOKEN_DIV);
					case 'e': {
						if (lexer->current - lexer->start > 2) {
							switch (lexer->start[2]) {
								case 'f': {
									if (lexer->current - lexer->start > 3) {
										switch (lexer->start[3]) {
											case 'g': return hir_checkKeyword(lexer, 4, 5, "lobal", HIR_TOKEN_DEFGLOBAL);
											case 'l': return hir_checkKeyword(lexer, 4, 4, "ocal", HIR_TOKEN_DEFLOCAL);
										}
									}
									break;
								}
							}
						}
						break;
					}
				}
			}
			break;
		}
		case 'e': {
			if (lexer->current - lexer->start > 1) {
				switch (lexer->start[1]) {
					case 'n': return hir_checkKeyword(lexer, 2, 6, "dscope", HIR_TOKEN_ENDSCOPE);
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
					case 'e': {
						if (lexer->current - lexer->start > 2) {
							switch (lexer->start[2]) {
								case 't': {
									if (lexer->current - lexer->start > 3) {
										switch (lexer->start[3]) {
											case 'g': return hir_checkKeyword(lexer, 4, 5, "lobal", HIR_TOKEN_GETGLOBAL);
											case 'l': return hir_checkKeyword(lexer, 4, 4, "ocal", HIR_TOKEN_GETLOCAL);
										}
									}
									break;
								}
							}
						}
						break;
					}
					case 'o': {
						if (hir_checkKeyword(lexer, 2, 5, "to_if", HIR_TOKEN_GOTO_IF) == HIR_TOKEN_GOTO_IF) {
							return HIR_TOKEN_GOTO_IF;
						}
						return hir_checkKeyword(lexer, 2, 2, "to", HIR_TOKEN_GOTO);
					}
					case 't': {
						if (hir_checkKeyword(lexer, 2, 2, "eq", HIR_TOKEN_GTEQ) == HIR_TOKEN_GTEQ) {
							return HIR_TOKEN_GTEQ;
						}
						return hir_checkKeyword(lexer, 1, 1, "t", HIR_TOKEN_GT);
					}
				}
			}
			break;
		}
		case 'j': {
			if (hir_checkKeyword(lexer, 1, 3, "ump", HIR_TOKEN_JUMP) == HIR_TOKEN_JUMP) {
				return HIR_TOKEN_JUMP;
			} else if (hir_checkKeyword(lexer, 1, 6, "ump_if", HIR_TOKEN_JUMP_IF) == HIR_TOKEN_JUMP_IF) {
				return HIR_TOKEN_JUMP_IF;
			}
			break;
		}
		case 'l': {
			if (lexer->current - lexer->start > 1) {
				if (hir_checkKeyword(lexer, 1, 3, "teq", HIR_TOKEN_LTEQ) == HIR_TOKEN_LTEQ) {
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
						if (lexer->current - lexer->start > 2) {
							switch (lexer->start[2]) {
								case 'g': return hir_checkKeyword(lexer, 2, 4, "gate", HIR_TOKEN_NEGATE);
								case 'q': return HIR_TOKEN_NEQ;
								case 'w': return hir_checkKeyword(lexer, 3, 5, "scope", HIR_TOKEN_NEWSCOPE);
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
		case 'r': return hir_checkKeyword(lexer, 1, 4, "turn", HIR_TOKEN_RETURN);
		case 's': {
			if (lexer->current - lexer->start > 1) {
				switch (lexer->start[1]) {
					case 'e': {
						if (lexer->current - lexer->start > 2) {
							switch (lexer->start[2]) {
								case 't': {
									if (lexer->current - lexer->start > 3) {
										switch (lexer->start[3]) {
											case 'g': return hir_checkKeyword(lexer, 4, 5, "lobal", HIR_TOKEN_SETGLOBAL);
											case 'l': return hir_checkKeyword(lexer, 4, 4, "ocal", HIR_TOKEN_SETLOCAL);
										}
									}
									break;
								}
							}
						}
						break;
					}
					case 'u': return hir_checkKeyword(lexer, 2, 1, "b", HIR_TOKEN_SUB);
				}
			}
			break;
		}
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
			case ':': return hir_label(lexer);
			case '$': return hir_identifier(lexer);
			case '"': return hir_string(lexer);
		}
	}

	return hir_errorToken(lexer, "unexpected character");
}

void hir_printToken(hir_Token *token)
{
	switch (token->type) {
		/* Literals */
		case HIR_TOKEN_LABEL: fputs("LABEL", stdout); break;
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
                case HIR_TOKEN_SETGLOBAL: fputs("SETGLOBAL", stdout); break;
                case HIR_TOKEN_GETGLOBAL: fputs("GETGLOBAL", stdout); break;
                case HIR_TOKEN_SETLOCAL: fputs("SETLOCAL", stdout); break;
                case HIR_TOKEN_DEFLOCAL: fputs("DEFLOCAL", stdout); break;
                case HIR_TOKEN_GETLOCAL: fputs("GETLOCAL", stdout); break;
                case HIR_TOKEN_NEWSCOPE: fputs("NEWSCOPE", stdout); break;
                case HIR_TOKEN_ENDSCOPE: fputs("ENDSCOPE", stdout); break;
		case HIR_TOKEN_JUMP: fputs("JUMP", stdout); break;
		case HIR_TOKEN_JUMP_IF: fputs("JUMP_IF", stdout); break;
		case HIR_TOKEN_BACK_JUMP: fputs("BACK_JUMP", stdout); break;
		case HIR_TOKEN_BACK_JUMP_IF: fputs("BACK_JUMP_IF", stdout); break;
		case HIR_TOKEN_GOTO: fputs("GOTO", stdout); break;
		case HIR_TOKEN_GOTO_IF: fputs("GOTO_IF", stdout); break;
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
