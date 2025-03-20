#include "chunk.h"
#include "chunk_loader.h"
#include "debug.h"
#include "vm.h"
#include "config.h"
#include "common.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if MEMWATCH
#include "memwatch.h"
#endif

static const char *help =
"Usage: hoshi [options]\n file"
"Run and disassemble compiled Hoshi bytecode.\n"
"One of -r or -d must be passed.\n\n"
"Options:\n"
"  -r, --run             Run the provided file.\n"
"  -d, --disassemble     Disassemble the input file.\n"
#if HOSHI_ENABLE_NOP_MODE
"  -N, --nop             A third *secret* mode which does nothing, used for testing purposes.\n"
#endif
"  -h, --help            Show this message.\n"
"Arguments:\n"
"  file                  Input file to run/disassemble."
"\n\n"
"Please report bugs to <https://github.com/emmathemartian/taiyo/issues>.";

typedef enum {
	NONE,
	RUN,
	DISASSEMBLE,
#if HOSHI_ENABLE_NOP_MODE
	NOP,
#endif
} Mode;

static Mode mode = NONE;
static char *inputFile = "";

#if HOSHI_ENABLE_NOP_MODE
static void nop();
#endif
static void runFile(const char *path);
static void disassembleFile(const char *path);

static void quit(int code)
{
	exit(code);
}

int main(int argc, char *argv[])
{
	static struct option longOptions[] = {
		{ "run",         no_argument, NULL, 'r' },
		{ "disassemble", no_argument, NULL, 'd' },
#if HOSHI_ENABLE_NOP_MODE
		{ "nop",         no_argument, NULL, 'N' },
#endif
		{ "help",        no_argument, NULL, 'h' },
		{ NULL,          0,           NULL, 0 }
	};

	if (argc == 1) {
		puts("error: no options specified\n\n");
		puts(help);
		quit(1);
	}

	int opt;
	while ((opt = getopt_long(
		argc,
		argv,
#if HOSHI_ENABLE_NOP_MODE
		":rdNh",
#else
		":rdH",
#endif
		longOptions,
		NULL)) != -1) {
		switch (opt) {
			/* Actions */
			case 'r':
				if (mode) {
					fputs("error: only one of -r or -d can be provided at a time.\n", stderr);
					quit(2);
				}
				mode = RUN;
				break;
			case 'd':
				if (mode) {
					fputs("error: only one of -r or -d can be provided at a time.\n", stderr);
					quit(2);
				}
				mode = DISASSEMBLE;
				break;
#if HOSHI_ENABLE_NOP_MODE
			case 'N':
				if (mode) {
					fputs("error: only one of -r, -d, or -N can be provided at a time.\n", stderr);
					quit(2);
				}
				mode = NOP;
				break;
#endif
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
		int len = strlen(*arg);
		inputFile = malloc(sizeof(char) * len);
		memcpy(inputFile, *arg, len);
#if HOSHI_ENABLE_NOP_MODE
	} else if (mode == NOP) {
		/* this is here so that -N can be passed without an input file */
#endif
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
		case DISASSEMBLE:
			disassembleFile(inputFile);
			break;
#if HOSHI_ENABLE_NOP_MODE
		case NOP:
			nop();
			break;
#endif
	}

	quit(0);
}

#if HOSHI_ENABLE_NOP_MODE
/* Used to get a baseline amount of bytes leaked (which hopefully is zero) */
static void nop()
{
	hoshi_VM vm;
	hoshi_initVM(&vm);
	hoshi_freeVM(&vm);
}
#endif

static void handleError(hoshi_VM *vm)
{
	quit(vm->exitCode);
}

static void runFile(const char *path)
{
	FILE *file = fopen(path, "rb");
	if (!file) {
		fprintf(stderr, "error: failed to open file: %s", path);
		quit(1);
	}

	/* Initialize VM */
	hoshi_VM vm;
	hoshi_initVM(&vm);
	vm.errorHandler = &handleError;

	/* Load chunk */
	hoshi_Chunk chunk;
	hoshi_initChunk(&chunk);
	bool readSuccess = hoshi_readChunkFromFile(&vm, &chunk, file, HOSHI_VERSION);
	fclose(file);
	if (!readSuccess) {
		fputs("error: failed to read chunk (see above error)\n", stderr);
		quit(1);
	}

	/* Run chunk */
	hoshi_runChunk(&vm, &chunk);

	/* Cleanup */
	int code = vm.exitCode;
	hoshi_freeChunk(&chunk);
	hoshi_freeVM(&vm);

	quit(code);
}

static void disassembleFile(const char *path)
{
	printf("Disassembling file: %s\n", path);
	puts("  | Reading");

	FILE *file = fopen(path, "rb");
	if (!file) {
		fprintf(stderr, "error: failed to read file: %s", path);
		quit(1);
	}

	/* Initialize VM */
	hoshi_VM vm;
	hoshi_initVM(&vm);
	vm.errorHandler = &handleError;

	/* Load chunk */
	puts("  | Loading");
	hoshi_Chunk chunk;
	hoshi_initChunk(&chunk);
	bool readSuccess = hoshi_readChunkFromFile(&vm, &chunk, file, HOSHI_VERSION);
	fclose(file);
	if (!readSuccess) {
		fputs("error: failed to read chunk (see above error)\n", stderr);
		quit(1);
	}

	/* Print disassembly */
	puts("  | Disasm");
	hoshi_disassembleChunk(&chunk, path);

	/* Cleanup */
	puts("  | Cleaning");
	hoshi_freeChunk(&chunk);
	hoshi_freeVM(&vm);
}
