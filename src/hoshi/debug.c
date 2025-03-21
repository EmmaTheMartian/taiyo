#ifndef __HOSHI_DEBUG_C__
#define __HOSHI_DEBUG_C__

#include "debug.h"
#include "chunk.h"
#include "config.h"
#include <stdint.h>
#include <stdio.h>

void hoshi_disassembleChunk(hoshi_Chunk *chunk, const char *name)
{
	printf("== %s ==\n", name);

#if HOSHI_DISASSEMBLER_ENABLE_RAW_CODE_DUMP
	puts("-- Raw Code Dump --");
	for (int offset = 0; offset < chunk->count; offset++) {
		printf("%d ", chunk->code[offset]);
	}
	puts("");
#endif

#if HOSHI_DISASSEMBLER_ENABLE_CONSTANT_DUMP
	puts("-- Constant Dump --");
	for (int id = 0; id < chunk->constants.count; id++) {
		printf("  %4d '", id);
		hoshi_printValue(chunk->constants.values[id]);
		puts("'");
	}
#endif

	printf("-- Instruction Dump (%d) --\n", chunk->count);
	for (int offset = 0; offset < chunk->count; ) {
		offset = hoshi_disassembleInstruction(chunk, offset);
	}
}

static int hoshi_simpleInstruction(const char *name, int offset)
{
	printf("%s\n", name);
	return offset + 1;
}

static int hoshi_constantInstruction(const char *name, hoshi_Chunk *chunk, int offset)
{
	uint8_t constant = chunk->code[offset + 1];
	if (constant >= chunk->constants.count) {
		/* print a different message to indicate out of range constants */
		printf("%-16s %4d [OOR]\n", name, constant);
	} else {
		printf("%-16s %4d '", name, constant);
		hoshi_printValue(chunk->constants.values[constant]);
		puts("'");
	}
	return offset + 2;
}

static int hoshi_longConstantInstruction(const char *name, hoshi_Chunk *chunk, int offset)
{
	int constant = (
		chunk->code[offset + 1] |
		(chunk->code[offset + 2] << 8) |
		(chunk->code[offset + 3] << 16)
	);
	if (constant >= chunk->constants.count) {
		/* print a different message to indicate out of range constants */
		printf("%-16s %4d [OOR]\n", name, constant);
	} else {
		printf("%-16s %4d '", name, constant);
		hoshi_printValue(chunk->constants.values[constant]);
		puts("'");
	}
	return offset + 4;
}

static int hoshi_longArgInstruction(const char *name, hoshi_Chunk *chunk, int offset)
{
	uint64_t arg = (
		chunk->code[offset + 1] |
		(chunk->code[offset + 2] << 8) |
		(chunk->code[offset + 3] << 16) |
		(chunk->code[offset + 4] << 24)
	);
	printf("%-16s      '%zu'", name, arg);
	return offset + 5;
}

static int hoshi_shortArgInstruction(const char *name, hoshi_Chunk *chunk, int offset)
{
	uint16_t arg = (
		chunk->code[offset + 1] |
		(chunk->code[offset + 2] << 8)
	);
	printf("%-16s      '%d'", name, arg);
	return offset + 3;
}

static int hoshi_byteArgInstruction(const char *name, hoshi_Chunk *chunk, int offset)
{
	printf("%-16s      '%d'\n", name, chunk->code[offset + 1]);
	return offset + 2;
}

int hoshi_disassembleInstruction(hoshi_Chunk *chunk, int offset)
{
	printf("%04d ", offset);

	int line = hoshi_getLine(chunk, offset);
	if (offset > 0 && line == hoshi_getLine(chunk, offset - 1)) {
		fputs("   | ", stdout);
	} else {
		printf("%4d ", line);
	}

	uint8_t instruction = chunk->code[offset];
	switch (instruction) {
		/* Stack ops */
		case HOSHI_OP_PUSH: return hoshi_byteArgInstruction("PUSH", chunk, offset);
		case HOSHI_OP_POP: return hoshi_simpleInstruction("POP", offset);
		case HOSHI_OP_CONSTANT: return hoshi_constantInstruction("CONSTANT", chunk, offset);
		case HOSHI_OP_CONSTANT_LONG: return hoshi_longConstantInstruction("CONSTANT_LONG", chunk, offset);
		case HOSHI_OP_TRUE: return hoshi_simpleInstruction("TRUE", offset);
		case HOSHI_OP_FALSE: return hoshi_simpleInstruction("FALSE", offset);
		case HOSHI_OP_NIL: return hoshi_simpleInstruction("NIL", offset);
		/* Variables */
		case HOSHI_OP_DEFGLOBAL: return hoshi_constantInstruction("DEFGLOBAL", chunk, offset);
		case HOSHI_OP_SETGLOBAL: return hoshi_constantInstruction("SETGLOBAL", chunk, offset);
		case HOSHI_OP_GETGLOBAL: return hoshi_constantInstruction("GETGLOBAL", chunk, offset);
		case HOSHI_OP_DEFLOCAL: return hoshi_byteArgInstruction("DEFLOCAL", chunk, offset);
		case HOSHI_OP_SETLOCAL: return hoshi_byteArgInstruction("SETLOCAL", chunk, offset);
		case HOSHI_OP_GETLOCAL: return hoshi_byteArgInstruction("GETLOCAL", chunk, offset);
		case HOSHI_OP_NEWSCOPE: return hoshi_simpleInstruction("NEWSCOPE", offset);
		case HOSHI_OP_ENDSCOPE: return hoshi_simpleInstruction("ENDSCOPE", offset);
		/* Control Flow */
		case HOSHI_OP_JUMP: return hoshi_shortArgInstruction("JUMP", chunk, offset);
		case HOSHI_OP_BACK_JUMP: return hoshi_shortArgInstruction("BACK_JUMP", chunk, offset);
		case HOSHI_OP_JUMP_IF: return hoshi_shortArgInstruction("JUMP_IF", chunk, offset);
		case HOSHI_OP_BACK_JUMP_IF: return hoshi_shortArgInstruction("BACK_JUMP_IF", chunk, offset);
		case HOSHI_OP_GOTO: return hoshi_longArgInstruction("GOTO", chunk, offset);
		case HOSHI_OP_GOTO_IF: return hoshi_longArgInstruction("GOTO_IF", chunk, offset);
		/* Math */
		case HOSHI_OP_ADD: return hoshi_simpleInstruction("ADD", offset);
		case HOSHI_OP_SUB: return hoshi_simpleInstruction("SUB", offset);
		case HOSHI_OP_MUL: return hoshi_simpleInstruction("MUL", offset);
		case HOSHI_OP_DIV: return hoshi_simpleInstruction("DIV", offset);
		case HOSHI_OP_NEGATE: return hoshi_simpleInstruction("NEGATE", offset);
		/* Booleans ops */
		case HOSHI_OP_NOT: return hoshi_simpleInstruction("NOT", offset);
		case HOSHI_OP_AND: return hoshi_simpleInstruction("AND", offset);
		case HOSHI_OP_OR: return hoshi_simpleInstruction("OR", offset);
		case HOSHI_OP_XOR: return hoshi_simpleInstruction("XOR", offset);
		/* Comparisons */
		case HOSHI_OP_EQ: return hoshi_simpleInstruction("EQ", offset);
		case HOSHI_OP_NEQ: return hoshi_simpleInstruction("NEQ", offset);
		case HOSHI_OP_GT: return hoshi_simpleInstruction("GT", offset);
		case HOSHI_OP_LT: return hoshi_simpleInstruction("LT", offset);
		case HOSHI_OP_GTEQ: return hoshi_simpleInstruction("GTEQ", offset);
		case HOSHI_OP_LTEQ: return hoshi_simpleInstruction("LTEQ", offset);
		/* String ops */
		case HOSHI_OP_CONCAT: return hoshi_simpleInstruction("CONCAT", offset);
		/* Misc */
		case HOSHI_OP_PRINT: return hoshi_simpleInstruction("PRINT", offset);
		case HOSHI_OP_RETURN: return hoshi_simpleInstruction("RETURN", offset);
		case HOSHI_OP_EXIT: return hoshi_simpleInstruction("EXIT", offset);
		default:
			printf("Unknown opcode: %d\n", instruction);
			return offset + 1;
	}
}

void hoshi_printStack(hoshi_VM *vm)
{
	fputs("stack: ", stdout);
	if (vm->stackTop == vm->stack) {
		puts("empty");
	} else {
		for (hoshi_Value *value = vm->stack; value < vm->stackTop; value++) {
			fputs("[ ", stdout);
			hoshi_printValue(*value);
			fputs(" ]", stdout);
		}
		puts("");
	}
}

#endif
