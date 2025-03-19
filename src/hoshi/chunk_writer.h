#ifndef __HOSHI_CHUNK_WRITER_H__
#define __HOSHI_CHUNK_WRITER_H__

#include "value.h"
#include "chunk.h"
#include "vm.h"
#include <stdio.h>

void hoshi_writeObjectToFile(hoshi_Object *object, FILE *file);
void hoshi_writeValueToFile(hoshi_Value *value, FILE *file);
void hoshi_writeChunkToFile(hoshi_VM *vm, hoshi_Chunk *chunk, FILE *file);

#endif
