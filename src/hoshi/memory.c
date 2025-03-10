#ifndef __HOSHI_MEMORY_C__
#define __HOSHI_MEMORY_C__

#include "memory.h"
#include "config.h"
#include <stdlib.h>

#if HOSHI_COUNT_LEAKED_BYTES
size_t hoshi_leakedBytes = 0;
#endif

void *hoshi_realloc(void *pointer, size_t oldSize, size_t newSize) {
	#if HOSHI_COUNT_LEAKED_BYTES
	hoshi_leakedBytes += newSize - oldSize;
	#endif

	if (newSize == 0) {
		free(pointer);
		return NULL;
	}

	void *result = realloc(pointer, newSize);
	if (result == NULL) {
		exit(1); // TODO: Throw a proper error message.
	}
	return result;
}

#endif
