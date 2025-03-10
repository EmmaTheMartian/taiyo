#ifndef __HOSHI_VM_C__
#define __HOSHI_VM_C__

#include "vm.h"
#include "chunk.h"
#include "value.h"
#include <stdarg.h>
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

void hoshi_panic(hoshi_VM *vm, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fputs("\n", stderr);

	size_t instruction = vm->ip - vm->chunk->code - 1;
	int line = hoshi_getLine(vm->chunk, instruction);
	fprintf(stderr, "[line %d] in script\n", line);
	hoshi_resetStack(vm);
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

hoshi_Value hoshi_peek(hoshi_VM *vm, int distance)
{
	return vm->stackTop[-1 - distance];
}

hoshi_InterpretResult hoshi_runNext(hoshi_VM *vm)
{
/* Macro shorthands. These get #undef'ed from existence after the for loop below. */
#define READ_BYTE() (*vm->ip++)
#define READ_CONSTANT() (vm->chunk->constants.values[READ_BYTE()])
#define BINARY_OP(valueType, op)\
	do { \
		if (!HOSHI_IS_NUMBER(hoshi_peek(vm, 0)) || !HOSHI_IS_NUMBER(hoshi_peek(vm, 1))) {\
			hoshi_panic(vm, "operands must be numbers."); \
			return HOSHI_INTERPRET_RUNTIME_ERROR; \
		} \
		double b = HOSHI_AS_NUMBER(hoshi_pop(vm)); \
		double a = HOSHI_AS_NUMBER(hoshi_pop(vm)); \
		hoshi_push(vm, valueType(a op b)); \
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
			case HOSHI_OP_PUSH: {
				hoshi_panic(vm, "push unimplemented.");
			// 	hoshi_Value it = READ_BYTE();
			// 	hoshi_push(vm, );
				break;
			}
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
			case HOSHI_OP_TRUE: hoshi_push(vm, HOSHI_BOOL(true)); break;
			case HOSHI_OP_FALSE: hoshi_push(vm, HOSHI_BOOL(false)); break;
			case HOSHI_OP_NIL: hoshi_push(vm, HOSHI_NIL()); break;
			case HOSHI_OP_ADD: BINARY_OP(HOSHI_NUMBER, +); break;
			case HOSHI_OP_SUB: BINARY_OP(HOSHI_NUMBER, -); break;
			case HOSHI_OP_MUL: BINARY_OP(HOSHI_NUMBER, *); break;
			case HOSHI_OP_DIV: BINARY_OP(HOSHI_NUMBER, /); break;
			case HOSHI_OP_NEGATE: {
				if (!HOSHI_IS_NUMBER(hoshi_peek(vm, 0))) {
					hoshi_panic(vm, "operand must be a number");
					return HOSHI_INTERPRET_RUNTIME_ERROR;
				}
				*(vm->stackTop - 1) = HOSHI_NUMBER(-HOSHI_AS_NUMBER(*(vm->stackTop - 1)));
				break;
			}
			case HOSHI_OP_NOT: {
				if (!HOSHI_IS_BOOL(hoshi_peek(vm, 0))) {
					hoshi_panic(vm, "operand must be a boolean");
					return HOSHI_INTERPRET_RUNTIME_ERROR;
				}
				*(vm->stackTop - 1) = HOSHI_BOOL(!HOSHI_AS_BOOL(*(vm->stackTop - 1)));
				break;
			}
			case HOSHI_OP_RETURN: {
				hoshi_printValue(hoshi_pop(vm));
				puts("");
				return HOSHI_INTERPRET_OK;
			}
			case HOSHI_OP_EXIT: {
				if (!HOSHI_IS_NUMBER(hoshi_peek(vm, 0))) {
					hoshi_panic(vm, "operand must be a number");
					return HOSHI_INTERPRET_RUNTIME_ERROR;
				}
				exit(HOSHI_AS_NUMBER(hoshi_pop(vm)));
			};
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
