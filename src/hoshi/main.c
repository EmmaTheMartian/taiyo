#include "hoshi.c"
#include "disassembler.c"
#include "hoshi.h"

int main(void) {
	hoshi_Chunk chunk;
	hoshi_initChunk(&chunk);

	int constant = hoshi_addConstant(&chunk, 1.2);
	hoshi_writeChunk(&chunk, HOSHI_OP_CONSTANT, 123);
	hoshi_writeChunk(&chunk, constant, 123);
	hoshi_writeChunk(&chunk, HOSHI_OP_RETURN, 123);

	int constant2 = hoshi_addConstant(&chunk, 1.5);
	hoshi_writeChunk(&chunk, HOSHI_OP_CONSTANT, 1234);
	hoshi_writeChunk(&chunk, constant2, 1234);
	hoshi_writeChunk(&chunk, HOSHI_OP_RETURN, 1234);

	hoshi_disassembleChunk(&chunk, "Disassembly");

	hoshi_freeChunk(&chunk);
	return 0;
}
