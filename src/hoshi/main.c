#include "chunk.h"
#include "vm.h"
#include <stdio.h>
#include <stdlib.h>

int main(const int argc, const char *argv[])
{
	if (argc == 1) {
		// repl(&vm);
	} else if (argc == 2) {
		// runFile(&vm, argv[1]);
	} else {
		fputs("Usage: hoshi [path]\n", stderr);
		exit(64);
	}

	return 0;
}
