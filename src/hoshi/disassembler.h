#ifndef __HOSHI_DISASSEMBLER_H__
#define __HOSHI_DISASSEMBLER_H__

#include "hoshi.h"

/* Config */

#ifndef HOSHI_DISASSEMBLER_ENABLE_RAW_CODE_DUMP
	/* Set to `1` to enable a raw dump of all bytecode instructions in the disassembler. */
	#define HOSHI_DISASSEMBLER_ENABLE_RAW_CODE_DUMP 0
#endif

#ifndef HOSHI_DISASSEMBLER_ENABLE_CONSTANT_DUMP
	/* Set to `1` to enable a raw dump of all constants in the disassembler. */
	#define HOSHI_DISASSEMBLER_ENABLE_CONSTANT_DUMP 0
#endif

/* Function signatures */

void hoshi_disassembleChunk(hoshi_Chunk *chunk, const char *name);
int hoshi_disassembleInstruction(hoshi_Chunk *chunk, int offset);

#endif
