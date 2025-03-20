#ifndef __HIR_LEXER_H__
#define __HIR_LEXER_H__

typedef enum {
	/* Literals */
	HIR_TOKEN_ID,
	HIR_TOKEN_NUMBER,
	HIR_TOKEN_STRING,
	HIR_TOKEN_TRUE,
	HIR_TOKEN_FALSE,
	HIR_TOKEN_NIL,
	/* Operations */
	HIR_TOKEN_PUSH,
	HIR_TOKEN_POP,
	HIR_TOKEN_ADD,
	HIR_TOKEN_SUB,
	HIR_TOKEN_MUL,
	HIR_TOKEN_DIV,
	HIR_TOKEN_NEGATE,
	HIR_TOKEN_DEFGLOBAL,
	HIR_TOKEN_SETGLOBAL,
	HIR_TOKEN_GETGLOBAL,
	HIR_TOKEN_SETLOCAL,
	HIR_TOKEN_DEFLOCAL,
	HIR_TOKEN_GETLOCAL,
	HIR_TOKEN_NEWSCOPE,
	HIR_TOKEN_ENDSCOPE,
	HIR_TOKEN_JUMP,
	HIR_TOKEN_BACK_JUMP,
	HIR_TOKEN_JUMP_IF,
	HIR_TOKEN_BACK_JUMP_IF,
	HIR_TOKEN_NOT,
	HIR_TOKEN_AND,
	HIR_TOKEN_OR,
	HIR_TOKEN_XOR,
	HIR_TOKEN_EQ,
	HIR_TOKEN_NEQ,
	HIR_TOKEN_GT,
	HIR_TOKEN_LT,
	HIR_TOKEN_GTEQ,
	HIR_TOKEN_LTEQ,
	HIR_TOKEN_CONCAT,
	HIR_TOKEN_PRINT,
	HIR_TOKEN_RETURN,
	HIR_TOKEN_EXIT,
	/* Misc */
	HIR_TOKEN_ERROR,
	HIR_TOKEN_EOF,
} hir_TokenType;

typedef struct {
	hir_TokenType type;
	const char *start;
	int length;
	int line;
} hir_Token;

typedef struct {
	const char *start;
	const char *current;
	int line;
} hir_Lexer;

void hir_initLexer(hir_Lexer *lexer, const char *source);
hir_Token hir_scanToken(hir_Lexer *lexer);
void hir_printToken(hir_Token *token);
void hir_lex(hir_Lexer *lexer);

#endif
