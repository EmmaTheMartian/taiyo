#ifndef __HOSHI_MEMORY_C__
#define __HOSHI_MEMORY_C__

#include "memory.h"
#include <stdlib.h>

void *hoshi_realloc(void *pointer, size_t oldSize, size_t newSize) {
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
