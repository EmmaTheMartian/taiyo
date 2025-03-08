/* HIR - A **very** minimalist front-end for Hoshi, literally Hoshi ASM. */

#include "compiler.h"
#include "../hoshi/debug.h"
#include <stdio.h>
#include <stdlib.h>

static void repl()
{
	hoshi_Chunk chunk;
	hoshi_VM vm;

	hoshi_initChunk(&chunk);
	hoshi_initVM(&vm);
	vm.chunk = &chunk;

	for (;;) {
		char line[1024];
		hoshi_printStack(&vm);
		printf("hir> ");
		if (!fgets(line, sizeof(line), stdin)) {
			puts("");
			break;
		}
		bool successful = hir_compileString(&chunk, line);
		if (successful) {
			puts("Valid syntax");
			hoshi_InterpretResult result = hoshi_runNext(&vm);
			switch (result) {
				case HOSHI_INTERPRET_OK: break;
				case HOSHI_INTERPRET_COMPILE_ERROR: break;
				case HOSHI_INTERPRET_RUNTIME_ERROR: break;
			}
		} else {
			puts("Invalid syntax");
		}
        }
}

static char *readFile(const char *path)
{
	FILE *file = fopen(path, "rb");
	if (file == NULL) {
		fprintf(stderr, "error: could not open file: %s\n", path);
		exit(74);
	}

	fseek(file, 0L, SEEK_END);
	size_t fileSize = ftell(file);
	rewind(file);

	char *buffer = malloc(fileSize + 1);
	if (buffer == NULL) {
		fprintf(stderr, "error: not enough memory to read `%s`", path);
		exit(74);
	}
	size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
	if (bytesRead < fileSize) {
		fprintf(stderr, "error: could not read file `%s`", path);
		exit(74);
	}
	buffer[bytesRead] = '\0';

	fclose(file);
	return buffer;
}

static void runFile(const char *path)
{
	char *source = readFile(path);

#if HIR_ENABLE_TOKEN_DUMP
	puts("== Token Dump ==");
	hir_Lexer lexer;
	hir_initLexer(&lexer, source);
	hir_lex(&lexer);
	puts("== End Token Dump ==");
#endif

	/* Compile code */
	hoshi_Chunk chunk;
	hoshi_initChunk(&chunk);
	hir_compileString(&chunk, source);

	/* Execute code */
	hoshi_VM vm;
	hoshi_initVM(&vm);
	hoshi_InterpretResult result = hoshi_runChunk(&vm, &chunk);

	/* Clean up */
	hoshi_freeChunk(&chunk);
	hoshi_freeVM(&vm);
	free(source);

	switch (result) {
		case HOSHI_INTERPRET_RUNTIME_ERROR: exit(70);
		case HOSHI_INTERPRET_COMPILE_ERROR: exit(65);
                default: break;
	}
}

int main(const int argc, const char *argv[])
{
	if (argc == 1) {
		repl();
	} else if (argc == 2) {
		runFile(argv[1]);
	} else {
		fputs("Usage: hoshi [path]\n", stderr);
		exit(64);
	}
}
