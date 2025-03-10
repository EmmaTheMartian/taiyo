#include "../common/fileio.c"
#include "chunk.h"
#include "debug.h"
#include "vm.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int exitCode = 0;

static const char *help =
"Usage: hoshi [options]\n file"
"Run and disassemble compiled Hoshi bytecode.\n"
"One of -r or -d must be passed.\n\n"
"Options:\n"
"  -r, --run             Run the provided file.\n"
"  -d, --disassemble     Disassemble the input file.\n"
"  -h, --help            Show this message.\n"
"Arguments:\n"
"  file                  Input file to run/disassemble."
"\n\n"
"Please report bugs to <https://github.com/emmathemartian/taiyo/issues>.";

typedef enum {
	NONE,
	RUN,
	DISASSEMBLE,
} Mode;

static Mode mode = NONE;
static char *inputFile = NULL;

static void runFile(const char *path);
static void disassembleFile(const char *path);

static void quit(int code)
{
#define CLEAN(pointer) { if (pointer == NULL) { free(pointer); } }
	CLEAN(inputFile);
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
	static struct option longOptions[] = {
		{ "run",         no_argument, NULL, 'r' },
		{ "disassemble", no_argument, NULL, 'd' },
		{ "help",        no_argument, NULL, 'h' },
		{ NULL,          0,           NULL, 0 }
	};

	if (argc == 1) {
		puts("error: no options specified\n\n");
		puts(help);
		quit(1);
	}

	int opt;
	while ((opt = getopt_long(argc, argv, ":rdh", longOptions, NULL)) != -1) {
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
	}

	quit(0);
}

static void runFile(const char *path)
{
	size_t fileSize;
	char *source = taiyoCommon_readFileWithSize(path, &fileSize);

	FILE *file = fopen(path, "rb");
	if (!file) {
		fprintf(stderr, "error: failed to open file: %s", path);
		quit(1);
	}

	/* Load chunk */
	hoshi_Chunk chunk;
	hoshi_initChunk(&chunk);
	bool readSuccess = hoshi_readChunkFromFile(&chunk, file);
	fclose(file);
	if (!readSuccess) {
		fputs("error: file had invalid header (are you sure it was a Hoshi file?)", stderr);
		quit(1);
	}

	/* Run chunk */
	hoshi_VM vm;
	hoshi_initVM(&vm);
	hoshi_runChunk(&vm, &chunk);

	/* Cleanup */
	int code = vm.exitCode;
	hoshi_freeChunk(&chunk);
	hoshi_freeVM(&vm);
	free(source);

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

	/* Load chunk */
	puts("  | Loading");
	hoshi_Chunk chunk;
	hoshi_initChunk(&chunk);
	bool readSuccess = hoshi_readChunkFromFile(&chunk, file);
	fclose(file);
	if (!readSuccess) {
		fputs("error: file had invalid header (are you sure it was a Hoshi file?)", stderr);
		quit(1);
	}

	/* Print disassembly */
	puts("  | Disasm");
	hoshi_disassembleChunk(&chunk, path);

	/* Cleanup */
	puts("  | Cleaning");
	hoshi_freeChunk(&chunk);
}
