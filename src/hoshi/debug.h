#ifndef __HOSHI_DEBUG_H__
#define __HOSHI_DEBUG_H__

#include "chunk.h"
#include "vm.h"

/* Function signatures */

void hoshi_disassembleChunk(hoshi_Chunk *chunk, const char *name);
int hoshi_disassembleInstruction(hoshi_Chunk *chunk, int offset);

void hoshi_printStack(hoshi_VM *vm);

#endif
