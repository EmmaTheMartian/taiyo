#ifndef __HIR_COMPILER_C__
#define __HIR_COMPILER_C__

#include "compiler.h"
#include "lexer.h"
#include "config.h"
#include "../hoshi/value.h"
#include "../hoshi/object.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#if HIR_ENABLE_PRINT_DISASSEMBLY
#include "../hoshi/debug.h"
#endif

static void hir_errorAt(hir_Parser *parser, hir_Token *token, const char *message)
{
	if (parser->panicMode) {
		return;
	}
	parser->panicMode = true;

	fprintf(stderr, "[line %d] Error", token->line);

	if (token->type == HIR_TOKEN_EOF) {
		fputs(" at end", stderr);
	} else if (token->type == HIR_TOKEN_ERROR) {
		// nop
	} else {
		fprintf(stderr, " at '%.*s'", token->length, token->start);
	}

	fprintf(stderr, ": %s\n", message);
	parser->hadError = true;
}

static void hir_error(hir_Parser *parser, const char *message)
{
	hir_errorAt(parser, &parser->previous, message);
}

static void hir_errorAtCurrent(hir_Parser *parser, const char *message)
{
	hir_errorAt(parser, &parser->current, message);
}

static void hir_advance(hir_Parser *parser, hir_Lexer *lexer)
{
	parser->previous = parser->current;

	for (;;) {
		parser->current = hir_scanToken(lexer);
#if HIR_ENABLE_PRINT_CODE
		fputs("token: ", stdout);
		hir_printToken(&parser->current);
#endif
		if (parser->current.type != HIR_TOKEN_ERROR) {
			break;
		}
		hir_errorAtCurrent(parser, parser->current.start);
	}
}

static void hir_consume(hir_Parser *parser, hir_Lexer *lexer, hir_TokenType type, const char *message)
{
	if (parser->current.type == type) {
		hir_advance(parser, lexer);
		return;
	}
	hir_errorAtCurrent(parser, message);
}

static void hir_emitByte(hir_Parser *parser, uint8_t byte)
{
	hoshi_writeChunk(parser->currentChunk, byte, parser->previous.line);
}

static void hir_emitBytes2(hir_Parser *parser, uint8_t byte1, uint8_t byte2)
{
	hir_emitByte(parser, byte1);
	hir_emitByte(parser, byte2);
}

static void hir_emitBytes3(hir_Parser *parser, uint8_t byte1, uint8_t byte2, uint8_t byte3)
{
	hir_emitByte(parser, byte1);
	hir_emitByte(parser, byte2);
	hir_emitByte(parser, byte3);
}

static void hir_endCompiler(hir_Parser *parser)
{
#if HIR_ENABLE_PRINT_DISASSEMBLY
	if (!parser->hadError) {
		hoshi_disassembleChunk(parser->currentChunk, "Code");
	}
#endif
}

static void hir_emitConstant(hir_Parser *parser, hoshi_Value value)
{
	hoshi_writeConstant(parser->currentChunk, value, parser->previous.line);
}

static void hir_number(hir_Parser *parser)
{
	hir_emitConstant(parser, HOSHI_NUMBER(strtod(parser->previous.start, NULL)));
}

static void hir_string(hoshi_ObjectTracker *tracker, hir_Parser *parser)
{
	hir_emitConstant(parser, HOSHI_OBJECT(hoshi_copyString(
		tracker,
		parser->previous.start + 1,
		parser->previous.length - 2
	)));
}

static void hir_expression(hoshi_ObjectTracker *tracker, hir_Parser *parser, hir_Lexer *lexer)
{
	hir_advance(parser, lexer);
	switch (parser->previous.type) {
		/* Values */
		// case HIR_TOKEN_DOLLAR:
		case HIR_TOKEN_NUMBER: hir_number(parser); break;
		case HIR_TOKEN_STRING: hir_string(tracker, parser); break;
		case HIR_TOKEN_TRUE: hir_emitByte(parser, HOSHI_OP_TRUE); break;
		case HIR_TOKEN_FALSE: hir_emitByte(parser, HOSHI_OP_FALSE); break;
		case HIR_TOKEN_NIL: hir_emitByte(parser, HOSHI_OP_NIL); break;
		/* Operations */
		case HIR_TOKEN_POP: hir_emitByte(parser, HOSHI_OP_POP); break;
		case HIR_TOKEN_ADD: hir_emitByte(parser, HOSHI_OP_ADD); break;
		case HIR_TOKEN_SUB: hir_emitByte(parser, HOSHI_OP_SUB); break;
		case HIR_TOKEN_MUL: hir_emitByte(parser, HOSHI_OP_MUL); break;
		case HIR_TOKEN_DIV: hir_emitByte(parser, HOSHI_OP_DIV); break;
		case HIR_TOKEN_NEGATE: hir_emitByte(parser, HOSHI_OP_NEGATE); break;
		case HIR_TOKEN_NOT: hir_emitByte(parser, HOSHI_OP_NOT); break;
		case HIR_TOKEN_AND: hir_emitByte(parser, HOSHI_OP_AND); break;
		case HIR_TOKEN_OR: hir_emitByte(parser, HOSHI_OP_OR); break;
		case HIR_TOKEN_XOR: hir_emitByte(parser, HOSHI_OP_XOR); break;
		case HIR_TOKEN_EQ: hir_emitByte(parser, HOSHI_OP_EQ); break;
		case HIR_TOKEN_NEQ: hir_emitByte(parser, HOSHI_OP_NEQ); break;
		case HIR_TOKEN_GT: hir_emitByte(parser, HOSHI_OP_GT); break;
		case HIR_TOKEN_LT: hir_emitByte(parser, HOSHI_OP_LT); break;
		case HIR_TOKEN_GTEQ: hir_emitByte(parser, HOSHI_OP_GTEQ); break;
		case HIR_TOKEN_LTEQ: hir_emitByte(parser, HOSHI_OP_LTEQ); break;
		case HIR_TOKEN_CONCAT: hir_emitByte(parser, HOSHI_OP_CONCAT); break;
		case HIR_TOKEN_RETURN: hir_emitByte(parser, HOSHI_OP_RETURN); break;
		case HIR_TOKEN_EXIT: hir_emitByte(parser, HOSHI_OP_EXIT); break;
		default:
			fprintf(stderr, "parser error: invalid token type for expression: %d (this error should never occur, please report it)\n", parser->previous.type);
        }
}

bool hir_compileString(hoshi_VM *vm, hoshi_Chunk *chunk, const char *string)
{
	hir_Lexer lexer;
	hir_Parser parser;

	hir_initLexer(&lexer, string);

	parser.hadError = false;
	parser.panicMode = false;
	parser.currentChunk = chunk;

	parser.previous.line = 1;
	parser.current.line = 1;

	/* FIXME: Temporary. Line numbers were breaking without this for whatever reason... */
	// hoshi_writeChunk(parser.currentChunk, HOSHI_OP_PUSH, 0);
	// hoshi_writeChunk(parser.currentChunk, -1, 0);
	// hoshi_writeChunk(parser.currentChunk, HOSHI_OP_POP, 0);

	hir_advance(&parser, &lexer);
	for (;;) {
		hir_expression(&vm->tracker, &parser, &lexer);
		if (parser.current.type == HIR_TOKEN_EOF) {
			break;
		}
	}
	// hir_consume(&parser, &lexer, HIR_TOKEN_EOF, "Expect end of file");

	hir_endCompiler(&parser);

	return !parser.hadError;
}

hoshi_InterpretResult hir_runString(hoshi_VM *vm, const char *string)
{
	hoshi_Chunk chunk;
	if (!hir_compileString(vm, &chunk, string)) {
		hoshi_freeChunk(&chunk);
		return HOSHI_INTERPRET_COMPILE_ERROR;
	}

	vm->chunk = &chunk;
	vm->ip = vm->chunk->code;

	hoshi_InterpretResult result = hoshi_runChunk(vm, &chunk);

	hoshi_freeChunk(&chunk);
	return result;
}

#endif
