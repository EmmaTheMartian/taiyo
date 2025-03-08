#ifndef __HOSHI_VM_C__
#define __HOSHI_VM_C__

#include "vm.h"
#include "chunk.h"
#include "value.h"
#include <stdio.h>
#include <stdlib.h>

#if HOSHI_ENABLE_TRACE_EXECUTION_DEBUGGING
#include "debug.h"
#endif

static void hoshi_resetStack(hoshi_VM *vm)
{
	vm->stackTop = vm->stack;
}

void hoshi_initVM(hoshi_VM *vm)
{
	hoshi_resetStack(vm);
}

void hoshi_freeVM(hoshi_VM *vm)
{
}

void hoshi_push(hoshi_VM *vm, hoshi_Value value)
{
	*vm->stackTop = value;
	vm->stackTop++;
}

hoshi_Value hoshi_pop(hoshi_VM *vm)
{
	vm->stackTop--;
	return *vm->stackTop;
}

hoshi_InterpretResult hoshi_runNext(hoshi_VM *vm)
{
/* Macro shorthands. These get #undef'ed from existence after the for loop below. */
#define READ_BYTE() (*vm->ip++)
#define READ_CONSTANT() (vm->chunk->constants.values[READ_BYTE()])
#define BINARY_OP(op)\
	do { \
		double b = hoshi_pop(vm); \
		double a = hoshi_pop(vm); \
		hoshi_push(vm, a op b); \
	} while (0)

	for (;;) {
#if HOSHI_ENABLE_TRACE_EXECUTION_DEBUGGING
		/* Print the stack */
		hoshi_printStack(vm);
		/* Print the instruction */
		hoshi_disassembleInstruction(vm->chunk, (int)(vm->ip - vm->chunk->code));
#endif

		uint8_t instruction;

		/* Here's a switch statement in its natural habitat. They're found in all interpreters somewhere. */
		switch (instruction = READ_BYTE()) {
			case HOSHI_OP_POP: {
				if (vm->stackTop == vm->stack) {
					fputs("error: attempted to pop but stack was empty.\n", stderr);
					exit(-1);
				}
				vm->stackTop--;
				break;
			}
			case HOSHI_OP_CONSTANT: {
				hoshi_Value constant = READ_CONSTANT();
				hoshi_push(vm, constant);
				break;
			}
			case HOSHI_OP_CONSTANT_LONG: {
				hoshi_Value constant = READ_CONSTANT();
				hoshi_push(vm, constant);
				break;
			}
			case HOSHI_OP_ADD: BINARY_OP(+); break;
			case HOSHI_OP_SUB: BINARY_OP(-); break;
			case HOSHI_OP_MUL: BINARY_OP(*); break;
			case HOSHI_OP_DIV: BINARY_OP(/); break;
			case HOSHI_OP_NEGATE: {
				*(vm->stackTop - 1) = -(*(vm->stackTop - 1));
				break;
			}
			case HOSHI_OP_RETURN: {
				hoshi_printValue(hoshi_pop(vm));
				puts("");
				return HOSHI_INTERPRET_OK;
			}
			case HOSHI_OP_EXIT: exit(hoshi_pop(vm));
		}
	}

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

hoshi_InterpretResult hoshi_runChunk(hoshi_VM *vm, hoshi_Chunk *chunk)
{
	vm->chunk = chunk;
	vm->ip = vm->chunk->code;
	return hoshi_runNext(vm);
}

#endif
