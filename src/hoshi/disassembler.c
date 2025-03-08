#ifndef __HOSHI_DISASSEMBLER_C__
#define __HOSHI_DISASSEMBLER_C__

#include "disassembler.h"
#include "hoshi.h"
#include <stdint.h>
#include <stdio.h>

void hoshi_disassembleChunk(hoshi_Chunk *chunk, const char *name)
{
	printf("== %s ==\n", name);

#if HOSHI_DISASSEMBLER_ENABLE_RAW_CODE_DUMP
	puts("-- Code Dump --");
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

	puts("-- Instruction Dump --");
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

int hoshi_disassembleInstruction(hoshi_Chunk *chunk, int offset)
{
	printf("%04d ", offset);

	int line = hoshi_getLine(chunk, offset);
	if (offset > 0 && line == hoshi_getLine(chunk, offset - 1)) {
		printf("   | ");
	} else {
		printf("%4d ", line);
	}

	uint8_t instruction = chunk->code[offset];
	switch (instruction) {
		case HOSHI_OP_CONSTANT:
			return hoshi_constantInstruction("CONSTANT", chunk, offset);
		case HOSHI_OP_RETURN:
			return hoshi_simpleInstruction("RETURN", offset);
		default:
			printf("Unknown opcode: %d\n", instruction);
			return offset + 1;
	}
}

#endif
