/* Configure Hoshi for debugging purposes */
#define HOSHI_ENABLE_TRACE_EXECUTION_DEBUGGING 1

#include "vm.h"

int main(void) {
	hoshi_VM vm;
	hoshi_Chunk chunk;

	hoshi_initVM(&vm);
	hoshi_initChunk(&chunk);

	hoshi_writeConstant(&chunk, 1.2, 123);
	hoshi_writeChunk(&chunk, HOSHI_OP_RETURN, 123);

	hoshi_writeConstant(&chunk, 1.5, 1234);
	hoshi_writeChunk(&chunk, HOSHI_OP_RETURN, 1234);

#if 0
	hoshi_disassembleChunk(&chunk, "Disassembly");
	puts("\n== Interpret ==");
#endif

	hoshi_interpret(&vm, &chunk);

	hoshi_freeChunk(&chunk);
	hoshi_freeVM(&vm);

	return 0;
}
