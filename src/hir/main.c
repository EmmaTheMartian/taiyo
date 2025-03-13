/* HIR - A **very** minimalist front-end for Hoshi, literally Hoshi ASM. */

#include "../common/fileio.c"
#include "../common/thirdparty/asprintf.h"
#include "compiler.h"
#include "../hoshi/debug.h"
#include "../hoshi/chunk_writer.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

static int exitCode = 0;

static const char *help =
"Usage: hir [options]\n file"
"Compile and execute HIR files.\n"
"One of -r, -c, or -t must be passed.\n\n"
"Options:\n"
"  -r, --run             Interpret the input file instead of compiling it.\n"
"  -c, --compile         Compile the input file.\n"
"  -t, --transpile       Transpile the input file to C.\n"
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
	TRANSPILE,
} Mode;

typedef enum {
	BACKEND_HOSHI,
	BACKEND_C,
} Backend;

static Mode mode = NONE;
static Backend backend = BACKEND_HOSHI;
static char *inputFile = NULL;
static char *outputFile = NULL;
static char *cc = NULL;
static char *ccFlags = NULL;
static bool printDisasm = false;

static void runFile(const char *path);
static void transpileFileToC(const char *inputFilePath, const char *outputFilePath, bool human);
static void compileFileToHoshi(const char *inputFilePath, const char *outputFilePath);
static void compileFileToC(const char *inputFilePath, const char *outputFilePath);

static void quit(int code)
{
#define CLEAN(pointer) { if (pointer == NULL) { free(pointer); } }

	CLEAN(outputFile);
	CLEAN(inputFile);
	CLEAN(cc);
	CLEAN(ccFlags);

	exit(code);
#undef CLEAN
}

static void setflag(char **flag)
{
	printf("setflag: `%s` to `%s`\n", *flag, optarg);
	/* Free the flag if it is already set. */
	if (*flag) {
		free(flag);
	}
	*flag = malloc(strlen(optarg) * sizeof(char));
	strcpy(*flag, optarg);
}

int main(int argc, char *argv[])
{
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
	while ((opt = getopt_long(argc, argv, ":rctdb:C:f:o:h", longopts, NULL)) != -1) {
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
			case 't':
				if (mode) {
					fputs("error: only one of -r, -c, or -t can be provided at a time.\n", stderr);
					quit(2);
				}
				mode = TRANSPILE;
				break;
			/* Config */
			case 'd':
				printDisasm = true;
				break;
			case 'b':
				if (memcmp(optarg, "hoshi", strlen(optarg)) == 0) {
					backend = BACKEND_HOSHI;
				} else if (memcmp(optarg, "c", strlen(optarg)) == 0) {
					backend = BACKEND_C;
				} else {
					fputs("error: no such backend (backends: hoshi, c)", stderr);
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

	/* Set default options */
	if (cc == NULL) { cc = "gcc"; }
	if (ccFlags == NULL) { ccFlags = ""; }

	/* Get input file */
	if (optind < argc) {
		char **arg = &argv[optind];
		int len = strlen(*arg);
		inputFile = malloc(sizeof(char) * len);
		memcpy(inputFile, *arg, len);
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
				case BACKEND_C:
					compileFileToC(inputFile, outputFile == NULL ? "a.out" : outputFile);
					break;
			}
			break;
		case TRANSPILE:
			transpileFileToC(inputFile, outputFile == NULL ? "out.c" : outputFile, true);
			break;
	}

	quit(0);
}

static char *getCompileCommand(const char *inputFilePath, const char *outputFilePath)
{
	char *command;
	asprintf(
		&command,
		"%s -o %s %s %s",
		cc,
		outputFilePath,
		ccFlags,
		inputFilePath
	);
	return command;
}

static void runFile(const char *path)
{
	char *source = taiyoCommon_readFile(path);

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

static void transpileFileToC(const char *inputFilePath, const char *outputFilePath, bool human)
{
	printf("--| %s\n", inputFilePath);
	char *source = taiyoCommon_readFile(inputFilePath);

#if HIR_ENABLE_TOKEN_DUMP
	puts("== Token Dump ==");
	hir_Lexer lexer;
	hir_initLexer(&lexer, source);
	hir_lex(&lexer);
	puts("== End Token Dump ==");
#endif

	/* Compile code */
	puts("  | Compiling");
	hoshi_Chunk chunk;
	hoshi_initChunk(&chunk);
	hir_compileString(&chunk, source);

	/* Disassembly */
	if (printDisasm) {
		hoshi_disassembleChunk(&chunk, "hoshi");
	}

	/* Write to C file */
	printf("  | Writing to %s\n", outputFilePath);
	FILE *outFile = fopen(outputFilePath, "wb");
	if (outFile == NULL) {
		fprintf(stderr, "Failed to open file: %s\n", outputFilePath);
		hoshi_freeChunk(&chunk);
		free(source);
		quit(74);
	}
	if (human) {
		hoshi_writeChunkToC(&chunk, outFile);
	} else {
		hoshi_writeChunkToCNonHuman(&chunk, outFile);
	}

	/* Clean up */
	fclose(outFile);
	hoshi_freeChunk(&chunk);
	free(source);
}

static void compileFileToHoshi(const char *inputFilePath, const char *outputFilePath)
{
	printf("--| %s\n", inputFilePath);
	char *source = taiyoCommon_readFile(inputFilePath);

#if HIR_ENABLE_TOKEN_DUMP
	puts("== Token Dump ==");
	hir_Lexer lexer;
	hir_initLexer(&lexer, source);
	hir_lex(&lexer);
	puts("== End Token Dump ==");
#endif

	/* Compile code */
	puts("  | Compiling");
	hoshi_Chunk chunk;
	hoshi_initChunk(&chunk);
	hir_compileString(&chunk, source);

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
	free(source);
}

static void compileFileToC(const char *inputFilePath, const char *outputFilePath)
{
	transpileFileToC(inputFilePath, "tmp.c", false);

	/* Compile the C file */
	puts("  | Compiling tmp.c");
	char *command = getCompileCommand("tmp.c", outputFilePath);
	printf("  | -> %s\n", command);
	if (system(command)) {
		fputs("error: compilation failed (see above error)\n", stderr);
		free(command);
		quit(1);
	}
	free(command);

	/* Remove C file */
	puts("  | Cleaning up");
	if (system("rm tmp.c")) {
		fputs("error: compilation failed (see above error)\n", stderr);
		quit(1);
	}
}
