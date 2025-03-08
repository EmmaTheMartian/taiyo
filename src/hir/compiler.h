#ifndef __HIR_COMPILER_H__
#define __HIR_COMPILER_H__

#include "../hoshi/chunk.h"
#include "../hoshi/vm.h"
#include "lexer.h"
#include <stdbool.h>

typedef struct {
	hir_Token current;
	hir_Token previous;
	bool hadError;
	bool panicMode;
	hoshi_Chunk *currentChunk;
} hir_Parser;

/* Compile the given string. */
bool hir_compileString(hoshi_Chunk *chunk, const char *string);

/* Compile and execute the given string. */
hoshi_InterpretResult hir_runString(hoshi_VM *vm, const char *string);

#endif
