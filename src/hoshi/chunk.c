#ifndef __HOSHI_CHUNK_C__
#define __HOSHI_CHUNK_C__

#include "chunk.h"
#include "memory.h"
#include "value.h"
#include "common.h"
#include "../common/binio.c"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void hoshi_initValueArray(hoshi_ValueArray *va)
{
	va->count = 0;
	va->capacity = 0;
	va->values = NULL;
}

void hoshi_freeValueArray(hoshi_ValueArray *va)
{
	HOSHI_FREE_ARRAY(hoshi_Value, va->values, va->capacity);
	hoshi_initValueArray(va);
}

void hoshi_writeValueArray(hoshi_ValueArray *va, hoshi_Value value)
{
	if (va->capacity < va->count + 1) {
		int oldCapacity = va->capacity;
		va->capacity = HOSHI_GROW_CAPACITY(oldCapacity);
		va->values = HOSHI_GROW_ARRAY(hoshi_Value, va->values, oldCapacity, va->capacity);
	}

	va->values[va->count] = value;
	va->count++;
}

void hoshi_initChunk(hoshi_Chunk *chunk)
{
	chunk->count = 0;
	chunk->capacity = 0;
	chunk->code = NULL;
	hoshi_initValueArray(&chunk->constants);
	chunk->lineCount = 0;
	chunk->lineCapacity = 0;
	chunk->lines = NULL;
}

void hoshi_freeChunk(hoshi_Chunk *chunk)
{
	HOSHI_FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
	hoshi_freeValueArray(&chunk->constants);
	HOSHI_FREE_ARRAY(int, chunk->lines, chunk->lineCapacity);
	hoshi_initChunk(chunk);
}

void hoshi_writeChunk(hoshi_Chunk *chunk, uint8_t byte, int line)
{
	if (chunk->capacity < chunk->count + 1) {
		int oldCapacity = chunk->capacity;
		chunk->capacity = HOSHI_GROW_CAPACITY(oldCapacity);
		chunk->code = HOSHI_GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
	}

	chunk->code[chunk->count] = byte;
	chunk->count++;

	/* If we are on the same line as the previous writeChunk call, we won't add a new hoshi_LineStart */
	if (chunk->lineCount > 0 && chunk->lines[chunk->lineCount - 1].line == line) {
		return;
	}

	/* Append a hoshi_LineStart */
	if (chunk->lineCapacity < chunk->lineCount + 1) {
		int oldCapacity = chunk->lineCapacity;
		chunk->lineCapacity = HOSHI_GROW_CAPACITY(oldCapacity);
		chunk->lines = HOSHI_GROW_ARRAY(hoshi_LineStart, chunk->lines, oldCapacity, chunk->lineCapacity);
	}

	chunk->lineCount++;
	hoshi_LineStart *lineStart = &chunk->lines[chunk->lineCount];
	lineStart->offset = chunk->count - 1;
	lineStart->line = line;
}

void hoshi_writeConstant(hoshi_Chunk *chunk, hoshi_Value value, int line)
{
	int index = hoshi_addConstant(chunk, value);
	if (index < 256) {
		hoshi_writeChunk(chunk, HOSHI_OP_CONSTANT, line);
		hoshi_writeChunk(chunk, (uint8_t)index, line);
	} else if (index < 0xFFFFFF) {
		hoshi_writeChunk(chunk, HOSHI_OP_CONSTANT_LONG, line);
		hoshi_writeChunk(chunk, (uint8_t)(index & 0xFF), line);
		hoshi_writeChunk(chunk, (uint8_t)((index >> 8) & 0xFF), line);
		hoshi_writeChunk(chunk, (uint8_t)((index >> 16) & 0xFF), line);
	} else {
		fprintf(stderr, "Attempted to create more constants than allowed in the chunk's constant pool.");
	}
}

int hoshi_addConstant(hoshi_Chunk *chunk, hoshi_Value value)
{
	hoshi_writeValueArray(&chunk->constants, value);
	return chunk->constants.count - 1;
}

int hoshi_getLine(hoshi_Chunk *chunk, int offset)
{
	int start = 0;
	int end = chunk->lineCount - 1;

	for (;;) {
		int mid = (start + end) / 2;
		hoshi_LineStart *line = &chunk->lines[mid];

		if (offset < line->offset) {
			end = mid - 1;
		} else if (mid == chunk->lineCount - 1 || offset < chunk->lines[mid + 1].offset) {
			return line->line;
		} else {
			start = mid + 1;
		}

		if (end < start) {
			return chunk->lines[start].line;
		}
	}
}

static int hoshi_decodeBytes(hoshi_Chunk *chunk, const char *source, size_t offset)
{
#define WRITE_AND_OFFSET() hoshi_writeChunk(chunk, source[offset], 1)
	/* first byte is assumed to be the instruction */
	hoshi_OpCode op = source[offset];
	hoshi_writeChunk(chunk, op, 1);

	/* some instructions have multiple arguments after them, which we need to decode too */
	switch (op) {
		case HOSHI_OP_CONSTANT:
			WRITE_AND_OFFSET();
			return 2;
		case HOSHI_OP_CONSTANT_LONG:
			WRITE_AND_OFFSET();
			WRITE_AND_OFFSET();
			WRITE_AND_OFFSET();
			return 3;
		default:
			return 1;
        }
#undef WRITE_AND_OFFSET
}

static const char hoshi_magicNumber[7] = { 0x7f, 'H', 'O', 'S', 'H', 'I', 0x7f };

bool hoshi_readChunkFromFile(hoshi_Chunk *chunk, FILE *file)
{
	/* I scope everything here so that local variables go out of their scope to keep the as small as possible while reading. */

	/* verify magic number */
	{
		char magic[7] = {
			binio_freadu8(file),
			binio_freadu8(file),
			binio_freadu8(file),
			binio_freadu8(file),
			binio_freadu8(file),
			binio_freadu8(file),
			binio_freadu8(file),
		};
		for (int i = 0; i < 7; i++) {
			if (hoshi_magicNumber[i] != magic[i]) {
				return false;
			}
		}
	}

	/* verify version */
	{
		uint16_t major = binio_freadu16(file);
		uint16_t minor = binio_freadu16(file);
		/* TODO: verify against... something */
	}

	/* read constants */
	{
		uint16_t constantCount = binio_freadu16(file);
		/* grow constant pool if needed */
		if (chunk->constants.capacity < constantCount + 1) {
			int oldCapacity = chunk->constants.capacity;
			chunk->constants.capacity = constantCount;
			chunk->constants.values = HOSHI_GROW_ARRAY(hoshi_Value, chunk->constants.values, oldCapacity, chunk->constants.capacity);
		}
		chunk->constants.count = constantCount;
		for (size_t i = 0; i < constantCount; i++) {
			hoshi_Value d = binio_freadd(file);
			chunk->constants.values[i] = d;
		}
	}

	/* read instructions */
	{
		uint16_t instructionCount = binio_freadu32(file);
		/* grow instruction array if needed */
		if (chunk->capacity < instructionCount + 1) {
			int oldCapacity = chunk->capacity;
			chunk->capacity = instructionCount;
			chunk->code = HOSHI_GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
		}
		chunk->count = instructionCount;
		for (size_t i = 0; i < instructionCount; i++) {
			chunk->code[i] = binio_freadu8(file);
		}
	}

	/* read line markers */
	{
		uint16_t lineCount = binio_freadu32(file);
		/* grow constant pool if needed */
		if (chunk->lineCapacity < lineCount + 1) {
			int oldCapacity = chunk->lineCapacity;
			chunk->lineCapacity = lineCount;
			chunk->lines = HOSHI_GROW_ARRAY(hoshi_LineStart, chunk->lines, oldCapacity, chunk->lineCapacity);
		}
		chunk->lineCount = lineCount;
		for (size_t i = 0; i < lineCount; i++) {
			chunk->lines[i].offset = binio_freadu16(file);
			chunk->lines[i].line = binio_freadu16(file);
		}
	}

	return true;
}

void hoshi_writeChunkToFile(hoshi_Chunk *chunk, FILE *file)
{
	/* magic number */
	fwrite(hoshi_magicNumber, 7, 1, file);
	/* version */
	binio_fputu16(HOSHI_VERSION_MAJOR, file);
	binio_fputu16(HOSHI_VERSION_MINOR, file);
	/* constant pool */
	binio_fputu16(chunk->constants.count, file);
	for (int i = 0; i < chunk->constants.count; i++) {
		binio_fputd(chunk->constants.values[i], file);
	}
	/* instructions */
	binio_fputu32(chunk->count, file);
	for (int i = 0; i < chunk->count; i++) {
		binio_fputu8(chunk->code[i], file);
	}
	/* line markers */
	binio_fputu32(chunk->lineCount, file);
	for (int i = 0; i < chunk->lineCount; i++) {
		binio_fputu16(chunk->lines[i].offset, file);
		binio_fputu16(chunk->lines[i].line, file);
	}
}

/* Writes the chunk to a C file which interprets the chunk. Weird, but it works. */
void hoshi_writeChunkToC(hoshi_Chunk *chunk, FILE *file)
{
	fprintf(file, "#include <hoshi/vm.h>\n#include <hoshi/chunk.h>\n\nstatic uint8_t bytecode[] = {\n");
	for (size_t i = 0; i < chunk->count; i++) {
		fprintf(file, "\t%d,\n", chunk->code[i]);
	}
	fprintf(file, "};\n\nstatic hoshi_Value constants[] = {\n");
	for (size_t i = 0; i < chunk->constants.count; i++) {
		fprintf(file, "\t%g,\n", chunk->constants.values[i]);
	}
	fprintf(file, "};\n\nstatic hoshi_LineStart lines[] = {\n");
	for (size_t i = 0; i < chunk->lineCount; i++) {
		fprintf(file, "\t{ %d, %d },\n", chunk->lines[i].offset, chunk->lines[i].line);
	}
	fprintf(
		file,
		"};\n\n"
		"static hoshi_Chunk chunk = {\n"
			"\t%d,\n"
			"\t%d,\n"
			"\t&bytecode[0],\n"
			"\t{ %d, %d, &constants[0] },\n"
			"\t%d,\n"
			"\t%d,\n"
			"\t&lines[0],\n"
		"};\n\n"
		"int main(void)\n"
		"{\n"
			"\thoshi_VM vm;\n"
			"\thoshi_initVM(&vm);\n"
			"\thoshi_runChunk(&vm, &chunk);\n"
			"\thoshi_freeVM(&vm);\n"
			"\treturn 0;\n"
		"}\n",
		chunk->count,
		chunk->count,
		chunk->constants.count,
		chunk->constants.count,
		chunk->lineCount,
		chunk->lineCount
	);
}

/* Same as hoshi_writeChunkToC but minimized, i.e, non-human readable */
void hoshi_writeChunkToCNonHuman(hoshi_Chunk *chunk, FILE *file)
{
	fprintf(file, "#include<hoshi/vm.h>\n#include<hoshi/chunk.h>\nstatic uint8_t a[]={");
	for (size_t i = 0; i < chunk->count; i++) {
		fprintf(file, "%d,", chunk->code[i]);
	}
	fprintf(file, "};static hoshi_Value b[]={");
	for (size_t i = 0; i < chunk->constants.count; i++) {
		fprintf(file, "%g,", chunk->constants.values[i]);
	}
	fprintf(file, "};static hoshi_LineStart c[]={");
	for (size_t i = 0; i < chunk->lineCount; i++) {
		fprintf(file, "{%d,%d},", chunk->lines[i].offset, chunk->lines[i].line);
	}
	fprintf(
		file,
		"};"
		"static hoshi_Chunk d={%d,%d,&a[0],{%d,%d,&b[0]},%d,%d,&c[0]};"
		"int main(void){"
			"hoshi_VM v;"
			"hoshi_initVM(&v);"
			"hoshi_runChunk(&v,&d);"
			"hoshi_freeVM(&v);"
			"return 0;"
		"}",
		chunk->count,
		chunk->count,
		chunk->constants.count,
		chunk->constants.count,
		chunk->lineCount,
		chunk->lineCount
	);
}

#endif
