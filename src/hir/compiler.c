#ifndef __HIR_COMPILER_C__
#define __HIR_COMPILER_C__

#include "compiler.h"
#include "lexer.h"
#include "config.h"
#include "../hoshi/value.h"
#include "../hoshi/object.h"
#include "../hoshi/common.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#if HIR_ENABLE_PRINT_DISASSEMBLY
#include "../hoshi/debug.h"
#endif

static void hir_errorAt(hir_Parser *parser, hir_Token *token, const char *message, ...)
{
	if (parser->panicMode) {
		return;
	}
	parser->panicMode = true;

	fprintf(stderr, "[line %d] error", token->line);

	if (token->type == HIR_TOKEN_EOF) {
		fputs(" at end", stderr);
	} else if (token->type == HIR_TOKEN_ERROR) {
		// nop
	} else {
		fprintf(stderr, " at '%.*s'", token->length, token->start);
	}

	fprintf(stderr, ": ");

	va_list args;
	va_start(args, message);
	fprintf(stderr, message, args);
	va_end(args);

	fprintf(stderr, "\n");

	parser->hadError = true;
}

static void hir_error(hir_Parser *parser, const char *message, ...)
{
	va_list args;
	va_start(args, message);
	hir_errorAt(parser, &parser->previous, message, args);
	va_end(args);
}

static void hir_errorAtCurrent(hir_Parser *parser, const char *message, ...)
{
	va_list args;
	va_start(args, message);
	hir_errorAt(parser, &parser->current, message);
	va_end(args);
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
	parser->bytePos++;
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

static void hir_emitLong(hir_Parser *parser, uint64_t value)
{
	hir_emitByte(parser, value & 0xFF);
	hir_emitByte(parser, (value >> 8) & 0xFF);
	hir_emitByte(parser, (value >> 16) & 0xFF);
	hir_emitByte(parser, (value >> 24) & 0xFF);
}

static void hir_endCompiler(hir_Parser *parser)
{
#if HIR_ENABLE_PRINT_DISASSEMBLY
	if (!parser->hadError) {
		hoshi_disassembleChunk(parser->currentChunk, "Code");
	}
#endif
}

static int hir_makeConstant(hoshi_VM *vm, hir_Parser *parser, hoshi_Value value)
{
	int constant = hoshi_addConstant(parser->currentChunk, value);
	if (constant >= UINT24_MAX) {
		hoshi_panic(vm, "too many constants in one chunk.");
		return 0;
	}
	return constant;
}

static void hir_emitConstant(hoshi_VM *vm, hir_Parser *parser, hoshi_Value value)
{
	int constant = hir_makeConstant(vm, parser, value);
	if (constant <= UINT8_MAX) {
		hir_emitBytes2(parser, HOSHI_OP_CONSTANT, constant);
	} else if (constant <= UINT24_MAX) {
		hir_emitByte(parser, HOSHI_OP_CONSTANT_LONG);
		hir_emitBytes3(
			parser,
			(uint8_t)(constant & 0xFF),
			(uint8_t)((constant >> 8) & 0xFF),
			(uint8_t)((constant >> 16) & 0xFF)
		);
	} else {
		/* should never happen, hir_makeConstant should panic first */
		hoshi_panic(vm, "too many constants in one chunk.");
	}
}

static bool hir_identifiersEqual(hir_Token *a, hir_Token *b)
{
	if (a->length != b->length) {
		return false;
	}
	return memcmp(a->start, b->start, a->length) == 0;
}

static uint8_t hir_globalId(hoshi_VM *vm, hir_Parser *parser, hir_Lexer *lexer)
{
	hir_consume(parser, lexer, HIR_TOKEN_ID, "expected identifier");

	/* This key gets freed at the end of compilation. */
	hoshi_ObjectString *key = hoshi_makeString(vm, false, (char *)parser->previous.start, parser->previous.length);

	return hoshi_addGlobal(vm, key);
}

static uint8_t hir_localId(hoshi_VM *vm, hir_Parser *parser, hir_Lexer *lexer, bool define)
{
	hir_consume(parser, lexer, HIR_TOKEN_ID, "expected identifier");

	/* Check if the local already exists*/
	for (int i = parser->currentCompiler->localCount - 1; i >= 0; i--) {
		hir_LocalVariable *local = &parser->currentCompiler->locals[i];
		if (hir_identifiersEqual(&parser->previous, &local->name)) {
			if (define) {
				hir_error(parser, "variable already exists in this scope.");
				return -1;
			}
			return parser->currentCompiler->locals[i].index;
		}
	}

	if (!define) {
		/* Variable doesn't exist and we aren't defining, so throw an error */
		hir_error(parser, "variable does not exist");
	}

	/* Add the variable */
	hir_LocalVariable *local = &parser->currentCompiler->locals[parser->currentCompiler->localCount++];
	local->name = parser->previous;
	local->depth = parser->currentCompiler->scopeDepth;
	local->index = hoshi_addLocal(vm);
	return local->index;
}

static uint64_t hir_label(hoshi_VM *vm, hir_Parser *parser, bool define)
{
	if (define) {
		hir_Label *label = &parser->currentCompiler->labels[parser->currentCompiler->labelCount++];
		label->name = parser->previous;
		label->pos = parser->bytePos;
		return parser->bytePos;
	} else {
		/* find the label by name
		 * TODO: I should use a hash map for this */
		hir_Token *name = &parser->previous;
		hir_Label *label;
		for (int i = 0; i < parser->currentCompiler->labelCount; i++) {
			label = &parser->currentCompiler->labels[i];
			if (hir_identifiersEqual(name, &label->name)) {
				return label->pos;
			}
		}
		hir_error(parser, "undefined label");
		return -1;
	}
}

static void hir_number(hoshi_VM *vm, hir_Parser *parser)
{
	hir_emitConstant(vm, parser, HOSHI_NUMBER(strtod(parser->previous.start, NULL)));
}

static void hir_string(hoshi_VM *vm, hir_Parser *parser)
{
	char *string = hoshi_formatString(vm, (char *)parser->previous.start, parser->previous.length);
	hir_emitConstant(vm, parser, HOSHI_OBJECT(hoshi_makeString(
		vm,
		true,
		string,
		strlen(string)
	)));
}

static void hir_expression(hoshi_VM *vm, hir_Parser *parser, hir_Lexer *lexer)
{
	hir_advance(parser, lexer);
	switch (parser->previous.type) {
		/* Values */
		case HIR_TOKEN_LABEL: hir_label(vm, parser, true); break;
		case HIR_TOKEN_NUMBER: hir_number(vm, parser); break;
		case HIR_TOKEN_STRING: hir_string(vm, parser); break;
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
		case HIR_TOKEN_DEFGLOBAL:
			hir_emitBytes2(parser, HOSHI_OP_DEFGLOBAL, hir_globalId(vm, parser, lexer));
			break;
		case HIR_TOKEN_SETGLOBAL:
			hir_emitBytes2(parser, HOSHI_OP_SETGLOBAL, hir_globalId(vm, parser, lexer));
			break;
		case HIR_TOKEN_GETGLOBAL:
			hir_emitBytes2(parser, HOSHI_OP_GETGLOBAL, hir_globalId(vm, parser, lexer));
			break;
		case HIR_TOKEN_DEFLOCAL:
			hir_emitBytes2(parser, HOSHI_OP_DEFLOCAL, hir_localId(vm, parser, lexer, true));
			break;
		case HIR_TOKEN_SETLOCAL:
			hir_emitBytes2(parser, HOSHI_OP_SETLOCAL, hir_localId(vm, parser, lexer, false));
			break;
		case HIR_TOKEN_GETLOCAL:
			hir_emitBytes2(parser, HOSHI_OP_GETLOCAL, hir_localId(vm, parser, lexer, false));
			break;
		case HIR_TOKEN_NEWSCOPE:
			hir_emitByte(parser, HOSHI_OP_NEWSCOPE);
			parser->currentCompiler->scopeDepth++;
			break;
		case HIR_TOKEN_ENDSCOPE:
			hir_emitByte(parser, HOSHI_OP_ENDSCOPE);
			parser->currentCompiler->scopeDepth--;

			/* Remove old locals */
			while (
				parser->currentCompiler->localCount > 0 &&
				parser->currentCompiler->locals[parser->currentCompiler->localCount - 1].depth > parser->currentCompiler->scopeDepth
			) {
				parser->currentCompiler->localCount--;
			}

			break;
		case HIR_TOKEN_JUMP: {
			long offset = strtol(parser->current.start, (char **)&parser->current.start[parser->current.length], 10);
			hir_advance(parser, lexer);
			hir_advance(parser, lexer);
			if (offset >= UINT16_MAX) {
				hir_error(parser, "attempting to jump too far (max is UINT16_MAX)");
			}
			hir_emitBytes3(parser, HOSHI_OP_JUMP, offset & 0xFF, (offset >> 8) & 0xFF);
			break;
		}
		case HIR_TOKEN_JUMP_IF: {
			long offset = strtol(parser->current.start, (char **)&parser->current.start[parser->current.length], 10);
			hir_advance(parser, lexer);
			hir_advance(parser, lexer);
			if (offset >= UINT16_MAX) {
				hir_error(parser, "attempting to jump too far (max is UINT16_MAX)");
			}
			hir_emitBytes3(parser, HOSHI_OP_JUMP_IF, offset & 0xFF, (offset >> 8) & 0xFF);
			break;
		}
		case HIR_TOKEN_BACK_JUMP: {
			long offset = -strtol(parser->current.start, (char **)&parser->current.start[parser->current.length], 10);
			hir_advance(parser, lexer);
			hir_advance(parser, lexer);
			if (offset >= UINT16_MAX) {
				hir_error(parser, "attempting to jump too far (max is UINT16_MAX)");
			}
			hir_emitBytes3(parser, HOSHI_OP_JUMP, offset & 0xFF, (offset >> 8) & 0xFF);
			break;
		}
		case HIR_TOKEN_BACK_JUMP_IF: {
			long offset = -strtol(parser->current.start, (char **)&parser->current.start[parser->current.length], 10);
			hir_advance(parser, lexer);
			hir_advance(parser, lexer);
			if (offset >= UINT16_MAX) {
				hir_error(parser, "attempting to jump too far (max is UINT16_MAX)");
			}
			hir_emitBytes3(parser, HOSHI_OP_JUMP_IF, offset & 0xFF, (offset >> 8) & 0xFF);
			break;
		}
		case HIR_TOKEN_GOTO: {
			hir_emitByte(parser, HOSHI_OP_GOTO);
			hir_emitLong(parser, hir_label(vm, parser, false));
			break;
		}
		case HIR_TOKEN_GOTO_IF: {
			hir_emitByte(parser, HOSHI_OP_GOTO_IF);
			hir_emitLong(parser, hir_label(vm, parser, false));
			break;
		}
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
		case HIR_TOKEN_PRINT: hir_emitByte(parser, HOSHI_OP_PRINT); break;
		case HIR_TOKEN_RETURN: hir_emitByte(parser, HOSHI_OP_RETURN); break;
		case HIR_TOKEN_EXIT: hir_emitByte(parser, HOSHI_OP_EXIT); break;
		default:
			hir_errorAtCurrent(parser, "invalid token type for expression: %d (this error should never happen, please report it)", parser->previous.type);
        }
}

static void hir_initCompiler(hir_Parser *parser, hir_Compiler *compiler)
{
	compiler->localCount = 0;
	compiler->scopeDepth = 0;
	compiler->labels = HOSHI_ALLOCATE(hir_Label, 256);
	parser->currentCompiler = compiler;
}

bool hir_compileString(hoshi_VM *vm, hoshi_Chunk *chunk, const char *string)
{
	hir_Lexer lexer;
	hir_Parser parser;

	hir_initLexer(&lexer, string);

	parser.bytePos = 0;
	parser.hadError = false;
	parser.panicMode = false;
	parser.currentChunk = chunk;
	hoshi_initTable(&parser.identifiers);
	hir_Compiler compiler;
	hir_initCompiler(&parser, &compiler);

	parser.previous.line = 1;
	parser.current.line = 1;

	hir_advance(&parser, &lexer);
	for (;;) {
		hir_expression(vm, &parser, &lexer);
		if (parser.current.type == HIR_TOKEN_EOF) {
			break;
		}
	}

	HOSHI_FREE_ARRAY(hir_Label, compiler.labels, compiler.labelCount);

	hir_endCompiler(&parser);
	hoshi_freeTable(&parser.identifiers);

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
