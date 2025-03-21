#ifndef __HOSHI_VM_C__
#define __HOSHI_VM_C__

#include "vm.h"
#include "chunk.h"
#include "config.h"
#include "hash_table.h"
#include "memory.h"
#include "value.h"
#include "object.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if HOSHI_ENABLE_TRACE_EXECUTION_DEBUGGING
#include "debug.h"
#endif

void hoshi_initScope(hoshi_Scope *scope)
{
	scope->localCount = 0;
}

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
	hoshi_initTable(&vm->strings);
	hoshi_initTable(&vm->globalNames);
	hoshi_initValueArray(&vm->globalValues);
	vm->localsTop = 0;
	vm->topScope = &vm->scopes[0];
	vm->errorHandler = NULL;

	for (int i = 0; i < HOSHI_LOCALS_SIZE; i++) {
		vm->locals[i] = (hoshi_LocalValue){ 0, HOSHI_NIL };
	}
}

extern size_t hoshi_leakedBytes;

void hoshi_freeVM(hoshi_VM *vm)
{
	hoshi_freeTable(&vm->strings);
	hoshi_freeTable(&vm->globalNames);
	hoshi_freeValueArray(&vm->globalValues);
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

	if (vm->errorHandler != NULL) {
		fprintf(stderr, "delegating to error handler\n");
		vm->errorHandler(vm);
	} else {
		fprintf(stderr, "hoshi_panic: warning: no error handler provided.\n");
	}
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

	hoshi_ObjectString *result = hoshi_makeString(vm, true, chars, length);
	hoshi_push(vm, HOSHI_OBJECT(result));
}

/* TODO: Rename to hoshi_run(hoshi_VM *vm) */
hoshi_InterpretResult hoshi_runNext(hoshi_VM *vm)
{
/* Macro shorthands. These get #undef'ed from existence after the for loop below. */
#define READ_BYTE() (*vm->ip++)
#define READ_SHORT() (*vm->ip += 2, (uint16_t)((vm->ip[-2] << 8) | vm->ip[-1]))
#define READ_LONG() (*vm->ip += 4, (uint64_t)((vm->ip[-4] << 24) | (vm->ip[-3] << 16) | (vm->ip[-2] << 8) | vm->ip[-1]))
#define READ_CONSTANT() (vm->chunk->constants.values[READ_BYTE()])
#define READ_STRING() HOSHI_AS_STRING(READ_CONSTANT())
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
			case HOSHI_OP_NIL: hoshi_push(vm, HOSHI_NIL); break;
			/* Variables */
			case HOSHI_OP_DEFGLOBAL: {
				vm->globalValues.values[READ_BYTE()] = hoshi_pop(vm);
				// hoshi_ObjectString *name = READ_STRING();
				// hoshi_tableSet(&vm->globals, name, hoshi_peek(vm, 0));
				// hoshi_pop(vm);
				break;
			}
			case HOSHI_OP_SETGLOBAL: {
				uint8_t index = READ_BYTE();
				if (HOSHI_IS_NIL(vm->globalValues.values[index])) {
					hoshi_panic(vm, "undefined variable: `%.*s`", vm->globalNames.entries[index].key->length, vm->globalNames.entries[index].key->chars);
					return HOSHI_INTERPRET_RUNTIME_ERROR;
				}
				vm->globalValues.values[READ_BYTE()] = hoshi_peek(vm, 0);
				// hoshi_ObjectString *name = READ_STRING();
				// if (hoshi_tableSet(&vm->globals, name, hoshi_peek(vm, 0))) {
				// 	hoshi_tableDelete(&vm->globals, name); /* delete the zombie value */
				// 	hoshi_panic(vm, "undefined variable: `%s`", name->chars);
				// 	return HOSHI_INTERPRET_RUNTIME_ERROR;
				// }
				break;
			}
			case HOSHI_OP_GETGLOBAL: {
				uint8_t index = READ_BYTE();
				hoshi_Value value = vm->globalValues.values[index];
				if (HOSHI_IS_NIL(value)) {
					hoshi_panic(vm, "undefined variable: `%.*s`", vm->globalNames.entries[index].key->length, vm->globalNames.entries[index].key->chars);
					return HOSHI_INTERPRET_RUNTIME_ERROR;
				}
				hoshi_push(vm, value);
				// hoshi_ObjectString *name = READ_STRING();
				// hoshi_Value value;
				// if (!hoshi_tableGet(&vm->globals, name, &value)) {
				// 	hoshi_panic(vm, "undefined variable: \"%.*s\"", name->length, name->chars);
				// 	return HOSHI_INTERPRET_RUNTIME_ERROR;
				// }
				// hoshi_push(vm, value);
				break;
			}
			case HOSHI_OP_DEFLOCAL: {
				uint8_t index = READ_BYTE();
				vm->locals[index].value = hoshi_pop(vm);
				vm->locals[index].depth = vm->scopes - vm->topScope;
				break;
			}
			case HOSHI_OP_SETLOCAL: {
				vm->locals[READ_BYTE()].value = hoshi_peek(vm, 0);
				break;
			}
			case HOSHI_OP_GETLOCAL: {
				hoshi_push(vm, vm->locals[READ_BYTE()].value);
				break;
			}
			case HOSHI_OP_NEWSCOPE: hoshi_pushScope(vm); break;
			case HOSHI_OP_ENDSCOPE: hoshi_popScope(vm); break;
			/* Control flow */
			case HOSHI_OP_JUMP: vm->ip += READ_SHORT(); break;
			case HOSHI_OP_BACK_JUMP: vm->ip -= READ_SHORT(); break;
			case HOSHI_OP_JUMP_IF: {
				hoshi_Value value = hoshi_pop(vm);
				uint16_t offset = READ_SHORT();
				if (HOSHI_IS_BOOL(value) && HOSHI_AS_BOOL(value)) {
					vm->ip += offset;
				}
				break;
			}
			case HOSHI_OP_BACK_JUMP_IF: {
				hoshi_Value value = hoshi_pop(vm);
				uint16_t offset = READ_SHORT();
				if (HOSHI_IS_BOOL(value) && HOSHI_AS_BOOL(value)) {
					vm->ip -= offset;
				}
				break;
			}
			case HOSHI_OP_GOTO: {
				vm->ip = &vm->chunk->code[READ_LONG()];
				break;
			}
			case HOSHI_OP_GOTO_IF: {
				hoshi_Value value = hoshi_pop(vm);
				uint64_t pos = READ_LONG();
				if (HOSHI_IS_BOOL(value) && HOSHI_AS_BOOL(value)) {
					vm->ip = &vm->chunk->code[pos];
				}
				break;
			}
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
			case HOSHI_OP_PRINT: {
				hoshi_printValue(hoshi_pop(vm));
				break;
			}
			case HOSHI_OP_RETURN: {
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
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP
#undef BINARY_BOOL_OP
}

hoshi_InterpretResult hoshi_runChunk(hoshi_VM *vm, hoshi_Chunk *chunk)
{
	vm->chunk = chunk;
	vm->ip = vm->chunk->code;

#if HOSHI_ENABLE_GLOBAL_NAME_DUMP
	puts("-- Global Dump --");
	for (int i = 0; i < vm->globalNames.count; i++) {
		if (vm->globalNames.entries[i].key != NULL) {
			printf("  [%d] = %.*s\n", i, vm->globalNames.entries[i].key->length, vm->globalNames.entries[i].key->chars);
		}
	}
#endif

	return hoshi_runNext(vm);
}

uint8_t hoshi_addGlobal(hoshi_VM *vm, hoshi_ObjectString *name)
{
	hoshi_Value index;
	if (hoshi_tableGet(&vm->globalNames, name, &index)) {
		return (uint8_t)HOSHI_AS_NUMBER(index);
	}

	uint8_t newIndex = (uint8_t)vm->globalValues.count;
	hoshi_writeValueArray(&vm->globalValues, HOSHI_NIL);
	hoshi_tableSet(&vm->globalNames, name, HOSHI_NUMBER((double)newIndex));
	return newIndex;
}

uint8_t hoshi_addLocal(hoshi_VM *vm)
{
	return vm->localsTop++;
}

void hoshi_pushScope(hoshi_VM *vm)
{
	vm->topScope++;
	hoshi_initScope(vm->topScope);
}

void hoshi_popScope(hoshi_VM *vm)
{
	int depth = vm->topScope - vm->scopes;
	// printf("popscope %d, %d\n", vm->topScope->localCount, depth);
	for (int i = 0; i < HOSHI_LOCALS_SIZE-1; i++) {
		// printf("  i: %d/%d, %d\n", i, HOSHI_LOCALS_SIZE-1, vm->locals[i].depth);
		if (vm->locals[i].depth > depth) {
			hoshi_LocalValue *value = &vm->locals[i];
			value->depth = 0;

			// if (HOSHI_IS_OBJECT(value->value)) {
			// 	hoshi_freeObject(HOSHI_AS_OBJECT(value->value));
			// }
			value->value = HOSHI_NIL;
			// value->value.as.number = 0;
			// value->value.type = HOSHI_TYPE_NIL;
		}
	}

	vm->localsTop -= vm->topScope->localCount;
	vm->topScope--;
}

#endif
