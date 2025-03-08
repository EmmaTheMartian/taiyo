#ifndef __HOSHI_VM_C__
#define __HOSHI_VM_C__

#include "vm.h"
#include <stdio.h>

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

hoshi_InterpretResult hoshi_interpret(hoshi_VM *vm, hoshi_Chunk *chunk)
{
	vm->chunk = chunk;
	vm->ip = vm->chunk->code;
	return hoshi_runNext(vm);
}

hoshi_InterpretResult hoshi_runNext(hoshi_VM *vm)
{
/* Macro shorthands. These get #undef'ed from existence after the for loop below. */
#define READ_BYTE() (*vm->ip++)
#define READ_CONSTANT() (vm->chunk->constants.values[READ_BYTE()])

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
			case HOSHI_OP_CONSTANT: {
				hoshi_Value constant = READ_CONSTANT();
				hoshi_push(vm, constant);
				break;
			}
			case HOSHI_OP_RETURN: {
				hoshi_printValue(hoshi_pop(vm));
				puts("");
				return HOSHI_INTERPRET_OK;
			}
		}
	}

#undef READ_BYTE
#undef READ_CONSTANT
}

#endif
