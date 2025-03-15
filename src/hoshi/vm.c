#ifndef __HOSHI_VM_C__
#define __HOSHI_VM_C__

#include "vm.h"
#include "chunk.h"
#include "memory.h"
#include "value.h"
#include "object.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#if HOSHI_ENABLE_TRACE_EXECUTION_DEBUGGING
#include "debug.h"
#endif

static void hoshi_resetStack(hoshi_VM *vm)
{
	vm->stackTop = vm->stack;
}

void hoshi_freeAllObjects(hoshi_VM *vm)
{
	hoshi_Object *object = vm->tracker.objects;
	while (object != NULL) {
		hoshi_Object *next = object->next;
		hoshi_freeObject(object);
		object = next;
	}
}

void hoshi_initVM(hoshi_VM *vm)
{
	hoshi_resetStack(vm);
	vm->exitCode = 0;
	vm->tracker.objects = NULL;
	hoshi_freeAllObjects(vm);
}

extern size_t hoshi_leakedBytes;

void hoshi_freeVM(hoshi_VM *vm)
{
	hoshi_freeAllObjects(vm);

	#if HOSHI_COUNT_LEAKED_BYTES
	printf("Leaked bytes: %zu\n", hoshi_leakedBytes);
	#endif
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

static void hoshi_concatenate(hoshi_VM *vm)
{
	hoshi_ObjectString *b = HOSHI_AS_STRING(hoshi_pop(vm));
	hoshi_ObjectString *a = HOSHI_AS_STRING(hoshi_pop(vm));

	int length = a->length + b->length;
	char *chars = HOSHI_ALLOCATE(char, length + 1);
	memcpy(chars, a->chars, a->length);
	memcpy(chars + a->length, b->chars, b->length);
	chars[length] = '\0';

	hoshi_ObjectString *result = hoshi_makeString(&vm->tracker, true, chars, length);
	hoshi_push(vm, HOSHI_OBJECT(result));
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
#define BINARY_BOOL_OP(op)\
	do { \
		if (!HOSHI_IS_BOOL(hoshi_peek(vm, 0)) || !HOSHI_IS_BOOL(hoshi_peek(vm, 1))) {\
			hoshi_panic(vm, "operands must be booleans."); \
			return HOSHI_INTERPRET_RUNTIME_ERROR; \
		} \
		bool b = HOSHI_AS_BOOL(hoshi_pop(vm)); \
		bool a = HOSHI_AS_BOOL(hoshi_pop(vm)); \
		hoshi_push(vm, HOSHI_BOOL(a op b)); \
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
			/* Stack ops */
			case HOSHI_OP_PUSH: {
				hoshi_panic(vm, "push unimplemented.");
			// 	hoshi_Value it = READ_BYTE();
			// 	hoshi_push(vm, );
				break;
			}
			case HOSHI_OP_POP: {
				if (vm->stackTop == vm->stack) {
					hoshi_panic(vm, "error: attempted to pop but stack was empty");
					return HOSHI_INTERPRET_RUNTIME_ERROR;
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
			/* Math */
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
			/* Boolean ops */
			case HOSHI_OP_NOT: {
				if (!HOSHI_IS_BOOL(hoshi_peek(vm, 0))) {
					hoshi_panic(vm, "operand must be a boolean");
					return HOSHI_INTERPRET_RUNTIME_ERROR;
				}
				*(vm->stackTop - 1) = HOSHI_BOOL(!HOSHI_AS_BOOL(*(vm->stackTop - 1)));
				break;
			}
			case HOSHI_OP_AND: BINARY_BOOL_OP(&&); break;
			case HOSHI_OP_OR: BINARY_BOOL_OP(||); break;
			case HOSHI_OP_XOR: BINARY_BOOL_OP(^); break;
			/* Comparisons */
			case HOSHI_OP_EQ: {
				hoshi_push(vm, HOSHI_BOOL(hoshi_valuesEqual(hoshi_pop(vm), hoshi_pop(vm))));
				break;
			}
			case HOSHI_OP_NEQ: {
				hoshi_push(vm, HOSHI_BOOL(!hoshi_valuesEqual(hoshi_pop(vm), hoshi_pop(vm))));
				break;
			}
			case HOSHI_OP_GT: BINARY_OP(HOSHI_BOOL, >); break;
			case HOSHI_OP_LT: BINARY_OP(HOSHI_BOOL, <); break;
			case HOSHI_OP_GTEQ: BINARY_OP(HOSHI_BOOL, >=); break;
			case HOSHI_OP_LTEQ: BINARY_OP(HOSHI_BOOL, <=); break;
			/* String ops */
			case HOSHI_OP_CONCAT: {
				if (!HOSHI_IS_STRING(hoshi_peek(vm, 0)) || !HOSHI_IS_STRING(hoshi_peek(vm, 1))) {
					hoshi_panic(vm, "operands must be strings");
				}
				hoshi_concatenate(vm);
				break;
			}
			/* Misc */
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
				vm->exitCode = HOSHI_AS_NUMBER(hoshi_pop(vm));
				return HOSHI_INTERPRET_OK;
			};
		}
	}

	return HOSHI_INTERPRET_OK;

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
#undef BINARY_BOOL_OP
}

hoshi_InterpretResult hoshi_runChunk(hoshi_VM *vm, hoshi_Chunk *chunk)
{
	vm->chunk = chunk;
	vm->ip = vm->chunk->code;
	return hoshi_runNext(vm);
}

#endif
