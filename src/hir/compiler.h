#ifndef __HIR_COMPILER_H__
#define __HIR_COMPILER_H__

#include "../hoshi/chunk.h"
#include "../hoshi/vm.h"
#include "config.h"
#include "lexer.h"
#include <stdbool.h>

typedef struct {
	hir_Token name;
	int depth;
	int index;
} hir_LocalVariable;

typedef struct {
	hir_Token name;
	int pos;
} hir_Label;

typedef struct {
	hir_LocalVariable locals[HIR_LOCAL_STACK_SIZE];
	int localCount;
	int scopeDepth;
	hir_Label *labels;
	int labelCount;
} hir_Compiler;

typedef struct {
	size_t bytePos;
	hir_Token current;
	hir_Token previous;
	bool hadError;
	bool panicMode;
	hoshi_Chunk *currentChunk;
	hoshi_Table identifiers;
	hir_Compiler *currentCompiler;
} hir_Parser;

/* Compile the given string. */
bool hir_compileString(hoshi_VM *vm, hoshi_Chunk *chunk, const char *string);

/* Compile and execute the given string. */
hoshi_InterpretResult hir_runString(hoshi_VM *vm, const char *string);

#endif
