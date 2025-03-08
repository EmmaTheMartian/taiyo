#ifndef __HOSHI_DISASSEMBLER_H__
#define __HOSHI_DISASSEMBLER_H__

#include "hoshi.h"

void hoshi_disassembleChunk(hoshi_Chunk *chunk, const char *name);
int hoshi_disassembleInstruction(hoshi_Chunk *chunk, int offset);

#endif
