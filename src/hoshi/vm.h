#ifndef __HOSHI_VM_H__
#define __HOSHI_VM_H__

#include "chunk.h"
#include "config.h"
#include "hash_table.h"
#include "memory.h"
#include "value.h"
#include <stdint.h>

struct hoshi_VM;

typedef void (*hoshi_ErrorHandler)(struct hoshi_VM *vm);

typedef enum {
	HOSHI_INTERPRET_OK,
	HOSHI_INTERPRET_COMPILE_ERROR,
	HOSHI_INTERPRET_RUNTIME_ERROR,
} hoshi_InterpretResult;

typedef struct {
	int localCount;
} hoshi_Scope;

typedef struct {
	int depth;
	hoshi_Value value;
} hoshi_LocalValue;

typedef struct hoshi_VM {
	/* Code */
	hoshi_Chunk *chunk;
	uint8_t *ip; /* Instruction Pointer */
	/* Stack */
	hoshi_Value stack[HOSHI_STACK_SIZE];
	hoshi_Value *stackTop;
	/* Strings */
	hoshi_Table strings;
	/* Globals */
	hoshi_Table globalNames;
	hoshi_ValueArray globalValues;
	/* Locals */
	hoshi_LocalValue locals[HOSHI_LOCALS_SIZE];
	int localsTop;
	hoshi_Scope scopes[HOSHI_MAX_SCOPE_DEPTH];
	hoshi_Scope *topScope;
	/* Exit */
	int exitCode;
	/* Memory management */
	hoshi_ObjectTracker tracker;
	/* Error handling */
	hoshi_ErrorHandler errorHandler;
} hoshi_VM;

void hoshi_initScope(hoshi_Scope *scope);
void hoshi_initVM(hoshi_VM *vm);
void hoshi_freeAllObjects(hoshi_VM *vm);
void hoshi_freeVM(hoshi_VM *vm);
void hoshi_panic(hoshi_VM *vm, const char *format, ...);
void hoshi_push(hoshi_VM *vm, hoshi_Value value);
hoshi_Value hoshi_pop(hoshi_VM *vm);
hoshi_InterpretResult hoshi_runNext(hoshi_VM *vm);
hoshi_InterpretResult hoshi_runChunk(hoshi_VM *vm, hoshi_Chunk *chunk);
uint8_t hoshi_addGlobal(hoshi_VM *vm, hoshi_ObjectString *name);
uint8_t hoshi_addLocal(hoshi_VM *vm);
void hoshi_pushScope(hoshi_VM *vm);
void hoshi_popScope(hoshi_VM *vm);

#endif
