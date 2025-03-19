#ifndef __HOSHI_CHUNK_LOADER_H__
#define __HOSHI_CHUNK_LOADER_H__

#include "memory.h"
#include "common.h"
#include "chunk.h"
#include "vm.h"
#include <stdio.h>

hoshi_Object *hoshi_readObjectFromFile(hoshi_VM *vm, FILE *file);
hoshi_Value hoshi_readValueFromFile(hoshi_VM *vm, hoshi_Value *value, FILE *file);
bool hoshi_readChunkFromFile(hoshi_VM *vm, hoshi_Chunk *chunk, FILE *file, hoshi_Version expectedVersion);

#endif
