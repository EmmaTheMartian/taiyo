/* HIR - A **very** minimalist front-end for Hoshi, literally Hoshi ASM. */

#include "compiler.h"
#include "config.h"
#include "../hoshi/debug.h"
#include "../hoshi/chunk_writer.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#if MEMWATCH
#include "memwatch.h"
#endif

static const char *help =
"Usage: hir [options]\n file"
"Compile and execute HIR files.\n"
"One of -r, or -c must be passed.\n\n"
"Options:\n"
"  -r, --run             Interpret the input file instead of compiling it.\n"
"  -c, --compile         Compile the input file.\n"
"  -b, --backend         Set the backend to compile to [default: hoshi].\n"
"  -C, --cc=<cc>         Set the C compiler [default: gcc].\n"
"  -f, --flags=<flags>   Provide flags to the C compiler.\n"
"  -o, --output=<path>   Set output path [default: a.out for -c, out.c for -t].\n"
"  -h, --help            Show this message.\n"
"Arguments:\n"
"  file                  Input file to compile/transpile/run."
"\n\n"
"Please report bugs to <https://github.com/emmathemartian/taiyo/issues>.";

typedef enum {
	NONE,
	RUN,
	COMPILE,
} Mode;

typedef enum {
	BACKEND_HOSHI,
} Backend;

static Mode mode = NONE;
static Backend backend = BACKEND_HOSHI;
static char *inputFile = "";
static char *outputFile = "";
static char *cc = "gcc";
static char *ccFlags = "";
static bool printDisasm = false;

static void runFile(const char *path);
static void compileFileToHoshi(const char *inputFilePath, const char *outputFilePath);

static void quit(int code)
{
#if MEMWATCH
	mwTerm();
#endif

	exit(code);
}

static void setflag(char **flag)
{
	/* Free the flag if it is already set. */
	if (*flag) {
		free(flag);
	}
	*flag = malloc(strlen(optarg) * sizeof(char));
	strcpy(*flag, optarg);
	free(flag);
}

int main(int argc, char *argv[])
{
#if MEMWATCH
	mwInit();
#endif
	static struct option longopts[] = {
		{ "run",           no_argument,       NULL, 'r' },
		{ "compile",       no_argument,       NULL, 'c' },
		{ "transpile",     no_argument,       NULL, 't' },
		{ "disassemble",   no_argument,       NULL, 'd' },
		{ "backend",       required_argument, NULL, 'b' },
		{ "cc",            required_argument, NULL, 'C' },
		{ "flags",         required_argument, NULL, 'f' },
		{ "output",        required_argument, NULL, 'o' },
		{ "help",          no_argument,       NULL, 'h' },
		{ NULL,            0,                 NULL, 0 }
	};

	if (argc == 1) {
		puts("error: no options specified\n\n");
		puts(help);
		quit(1);
	}

	int opt;
	while ((opt = getopt_long(argc, argv, ":rcdb:C:f:o:h", longopts, NULL)) != -1) {
		switch (opt) {
			/* Actions */
			case 'r':
				if (mode) {
					fputs("error: only one of -r, -c, or -t can be provided at a time.\n", stderr);
					quit(2);
				}
				mode = RUN;
				break;
			case 'c':
				if (mode) {
					fputs("error: only one of -r, -c, or -t can be provided at a time.\n", stderr);
					quit(2);
				}
				mode = COMPILE;
				break;
			/* Config */
			case 'd':
				printDisasm = true;
				break;
			case 'b':
				if (memcmp(optarg, "hoshi", strlen(optarg)) == 0) {
					backend = BACKEND_HOSHI;
				} else {
					fputs("error: no such backend (backends: hoshi)", stderr);
					quit(1);
				}
				break;
			case 'C':
				setflag(&cc);
				break;
			case 'f':
				setflag(&ccFlags);
				break;
			case 'o':
				setflag(&outputFile);
				break;
			/* Help */
			case 'h':
				puts(help);
				quit(0);
			/* Errors */
			case '?':
				printf("error: unknown option: %c\n", optopt);
				quit(1);
			case ':':
				printf("error: missing arg for: %c\n", optopt);
				quit(1);
			default:
				printf("error: getopt returned unhandled opt: `%c`\n", opt);
				quit(1);
		}
	}

	/* Get input file */
	if (optind < argc) {
		char **arg = &argv[optind];
		int len = strlen(*arg) + 1;
		inputFile = malloc(sizeof(char) * len);
		memcpy(inputFile, *arg, len);
		inputFile[len + 1] = '\0';
	} else {
		fputs("error: no input file specified.\n", stderr);
		quit(2);
	}

	switch (mode) {
		case NONE:
			fputs("error: no mode specified. pass --help for usage.\n", stderr);
			quit(2);
		case RUN:
			runFile(inputFile);
			break;
		case COMPILE:
			switch (backend) {
				case BACKEND_HOSHI:
					compileFileToHoshi(inputFile, outputFile == NULL ? "out.hoshi" : outputFile);
					break;
			}
			break;
	}

	quit(0);
}

static char *hir_readFile(const char *filePath)
{
	FILE *file = fopen(filePath, "rb");
	if (file == NULL) {
		fprintf(stderr, "error: could not open file: %s\n", filePath);
		quit(74);
	}

	rewind(file);
	fseek(file, 0L, SEEK_END);
	size_t fileSize = ftell(file);
	rewind(file);

	char *buffer = malloc(sizeof(char) * fileSize);
	if (buffer == NULL) {
		fprintf(stderr, "error: not enough memory to read `%s`", filePath);
		quit(74);
	}
	size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
	if (bytesRead < fileSize) {
		fprintf(stderr, "error: could not read file `%s`", filePath);
		quit(74);
	}
	buffer[fileSize] = '\0';

	fclose(file);
	return buffer;
}

static void runFile(const char *path)
{
	char *source = hir_readFile(path);

#if HIR_ENABLE_TOKEN_DUMP
	puts("== Token Dump ==");
	hir_Lexer lexer;
	hir_initLexer(&lexer, source);
	hir_lex(&lexer);
	puts("== End Token Dump ==");
#endif

	/* Initialize VM */
	hoshi_VM vm;
	hoshi_initVM(&vm);

	/* Compile code */
	hoshi_Chunk chunk;
	hoshi_initChunk(&chunk);
	hir_compileString(&vm, &chunk, source);

	/* Execute code */
	hoshi_InterpretResult result = hoshi_runChunk(&vm, &chunk);

	/* Clean up */
	int code = vm.exitCode;
	hoshi_freeChunk(&chunk);
	hoshi_freeVM(&vm);
	free(source);

	switch (result) {
		case HOSHI_INTERPRET_RUNTIME_ERROR: quit(70);
		case HOSHI_INTERPRET_COMPILE_ERROR: quit(65);
                default: quit(code);
	}
}

static void compileFileToHoshi(const char *inputFilePath, const char *outputFilePath)
{
	printf("--| %s\n", inputFilePath);
	char *source = hir_readFile(inputFilePath);

#if HIR_ENABLE_TOKEN_DUMP
	puts("== Token Dump ==");
	hir_Lexer lexer;
	hir_initLexer(&lexer, source);
	hir_lex(&lexer);
	puts("== End Token Dump ==");
#endif

	/* Initialize VM */
	hoshi_VM vm;
	hoshi_initVM(&vm);

	/* Compile code */
	puts("  | Compiling");
	hoshi_Chunk chunk;
	hoshi_initChunk(&chunk);
	hir_compileString(&vm, &chunk, source);

	/* Disassembly */
	if (printDisasm) {
		hoshi_disassembleChunk(&chunk, "hoshi");
	}

	/* Write to Hoshi file */
	printf("  | Writing to %s\n", outputFilePath);
	FILE *outFile = fopen(outputFilePath, "wb");
	if (outFile == NULL) {
		fprintf(stderr, "Failed to open file: %s\n", outputFilePath);
		hoshi_freeChunk(&chunk);
		free(source);
		quit(74);
	}
	hoshi_writeChunkToFile(&chunk, outFile);

	/* Clean up */
	puts("  | Cleaning up");
	fclose(outFile);
	hoshi_freeChunk(&chunk);
	hoshi_freeVM(&vm);
	free(source);
}
