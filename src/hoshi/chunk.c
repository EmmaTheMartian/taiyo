#ifndef __HOSHI_CHUNK_C__
#define __HOSHI_CHUNK_C__

#include "chunk.h"
#include "memory.h"
#include "value.h"
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
	/* Add the first line line */
	// HOSHI_GROW_ARRAY(hoshi_LineStart, chunk->code, 0, 8);
	// chunk->lines[0].offset = 0;
	// chunk->lines[0].line = 1;
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

#endif
