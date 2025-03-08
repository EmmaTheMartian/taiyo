#ifndef __HOSHI_VM_H__
#define __HOSHI_VM_H__

#include "chunk.h"
#include "config.h"

/* Enumerations */

typedef enum {
	HOSHI_INTERPRET_OK,
	HOSHI_INTERPRET_COMPILE_ERROR,
	HOSHI_INTERPRET_RUNTIME_ERROR,
} hoshi_InterpretResult;

/* Structures */

typedef struct {
	hoshi_Chunk *chunk;
	uint8_t *ip; /* Instruction Pointer */
	hoshi_Value stack[HOSHI_STACK_SIZE];
	hoshi_Value *stackTop;
} hoshi_VM;

/* Function signatures */

void hoshi_initVM(hoshi_VM *vm);
void hoshi_freeVM(hoshi_VM *vm);
void hoshi_push(hoshi_VM *vm, hoshi_Value value);
hoshi_Value hoshi_pop(hoshi_VM *vm);
hoshi_InterpretResult hoshi_runNext(hoshi_VM *vm);
hoshi_InterpretResult hoshi_runChunk(hoshi_VM *vm, hoshi_Chunk *chunk);

#endif
