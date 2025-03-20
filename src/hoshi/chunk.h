#ifndef __HOSHI_CHUNK_H__
#define __HOSHI_CHUNK_H__

#include "value.h"
#include "memory.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

typedef enum {
	/* Stack ops */
	HOSHI_OP_PUSH,
	HOSHI_OP_POP,
	HOSHI_OP_CONSTANT,
	HOSHI_OP_CONSTANT_LONG,
	HOSHI_OP_TRUE,
	HOSHI_OP_FALSE,
	HOSHI_OP_NIL,
	/* Variables */
	/*       Globals */
	HOSHI_OP_DEFGLOBAL,
	HOSHI_OP_SETGLOBAL,
	HOSHI_OP_GETGLOBAL,
	/*       Locals */
	HOSHI_OP_DEFLOCAL,
	HOSHI_OP_SETLOCAL,
	HOSHI_OP_GETLOCAL,
	/*       Scopes */
	HOSHI_OP_NEWSCOPE,
	HOSHI_OP_ENDSCOPE,
	/* Math */
	HOSHI_OP_ADD,
	HOSHI_OP_SUB,
	HOSHI_OP_MUL,
	HOSHI_OP_DIV,
	HOSHI_OP_NEGATE,
	/* Boolean ops */
	HOSHI_OP_NOT,
	HOSHI_OP_AND,
	HOSHI_OP_OR,
	HOSHI_OP_XOR,
	/* Comparisons */
	HOSHI_OP_EQ,
	HOSHI_OP_NEQ,
	HOSHI_OP_GT,
	HOSHI_OP_LT,
	HOSHI_OP_GTEQ,
	HOSHI_OP_LTEQ,
	/* String ops */
	HOSHI_OP_CONCAT,
	/* Misc */
	HOSHI_OP_PRINT,
	HOSHI_OP_RETURN,
	HOSHI_OP_EXIT,
} hoshi_OpCode;

typedef struct {
	int count;
	int capacity;
	hoshi_Value *values;
} hoshi_ValueArray;

typedef struct {
	int offset;
	int line;
} hoshi_LineStart;

typedef struct {
	int count;
	int capacity;
	uint8_t *code;
	hoshi_ValueArray constants;
	int lineCount;
	int lineCapacity;
	hoshi_LineStart *lines;
} hoshi_Chunk;

static const char hoshi_magicNumber[7] = { 0x7f, 'H', 'O', 'S', 'H', 'I', 0x7f };

void hoshi_initValueArray(hoshi_ValueArray *va);
void hoshi_freeValueArray(hoshi_ValueArray *va);
void hoshi_writeValueArray(hoshi_ValueArray *va, hoshi_Value value);
void hoshi_initChunk(hoshi_Chunk *chunk);
void hoshi_freeChunk(hoshi_Chunk *chunk);
void hoshi_writeChunk(hoshi_Chunk *chunk, uint8_t byte, int line);
void hoshi_writeConstant(hoshi_Chunk *chunk, hoshi_Value value, int line);
int hoshi_addConstant(hoshi_Chunk *chunk, hoshi_Value value);
int hoshi_getLine(hoshi_Chunk *chunk, int instruction);

#endif
