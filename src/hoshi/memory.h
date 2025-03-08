#ifndef __HOSHI_MEMORY_H__
#define __HOSHI_MEMORY_H__

#include <stddef.h>

void *hoshi_realloc(void *pointer, size_t oldSize, size_t newSize);

#endif
