#ifndef __HOSHI_CHUNK_LOADER_H__
#define __HOSHI_CHUNK_LOADER_H__

#include "memory.h"
#include "common.h"
#include "chunk.h"
#include <stdio.h>

hoshi_Object *hoshi_readObjectFromFile(hoshi_ObjectTracker *tracker, FILE *file);
hoshi_Value hoshi_readValueFromFile(hoshi_ObjectTracker *tracker, hoshi_Value *value, FILE *file);
bool hoshi_readChunkFromFile(hoshi_ObjectTracker *tracker, hoshi_Chunk *chunk, FILE *file, hoshi_Version expectedVersion);

#endif
