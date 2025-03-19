#ifndef __HOSHI_VM_H__
#define __HOSHI_VM_H__

#include "chunk.h"
#include "config.h"
#include "hash_table.h"
#include "memory.h"
#include "value.h"
#include <stdint.h>

typedef enum {
	HOSHI_INTERPRET_OK,
	HOSHI_INTERPRET_COMPILE_ERROR,
	HOSHI_INTERPRET_RUNTIME_ERROR,
} hoshi_InterpretResult;

typedef struct {
	hoshi_Chunk *chunk;
	uint8_t *ip; /* Instruction Pointer */
	hoshi_Value stack[HOSHI_STACK_SIZE];
	hoshi_Value *stackTop;
	hoshi_Table strings;
	hoshi_Table globalNames;
	hoshi_ValueArray globalValues;
	int exitCode;
	hoshi_ObjectTracker tracker;
} hoshi_VM;

void hoshi_initVM(hoshi_VM *vm);
void hoshi_freeAllObjects(hoshi_VM *vm);
void hoshi_freeVM(hoshi_VM *vm);
void hoshi_panic(hoshi_VM *vm, const char *format, ...);
void hoshi_push(hoshi_VM *vm, hoshi_Value value);
hoshi_Value hoshi_pop(hoshi_VM *vm);
hoshi_InterpretResult hoshi_runNext(hoshi_VM *vm);
hoshi_InterpretResult hoshi_runChunk(hoshi_VM *vm, hoshi_Chunk *chunk);
uint8_t hoshi_addGlobal(hoshi_VM *vm, hoshi_ObjectString *name);

#endif
