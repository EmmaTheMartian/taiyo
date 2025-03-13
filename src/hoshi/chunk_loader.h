#ifndef __HOSHI_CHUNK_LOADER_H__
#define __HOSHI_CHUNK_LOADER_H__

#include "value.h"
#include "chunk.h"
#include <stdio.h>

hoshi_Object *hoshi_readObjectFromFile(FILE *file);
hoshi_Value hoshi_readValueFromFile(hoshi_Value *value, FILE *file);
bool hoshi_readChunkFromFile(hoshi_Chunk *chunk, FILE *file);

#endif
