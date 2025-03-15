#ifndef __HOSHI_MEMORY_C__
#define __HOSHI_MEMORY_C__

#include "memory.h"
#include "config.h"
#include <stdlib.h>

#if HOSHI_TRACE_ALLOCATIONS
#include <stdio.h>
#endif

#if MEMWATCH
#include "memwatch.h"
#endif

/* This file contains a **lot** of code toggled at compile-time, so instead of writing tons of #ifs everywhere, I'll just make a simple macro instead.
 * These get #undef'd at the end of the file */
#if HOSHI_COUNT_LEAKED_BYTES
	#define WHEN_COUNT(...) __VA_ARGS__
#else
	#define WHEN_COUNT(...) ;
#endif
#if HOSHI_TRACE_ALLOCATIONS
	#define WHEN_TRACE(...) __VA_ARGS__
#else
	#define WHEN_TRACE(...) ;
#endif
#if HOSHI_COUNT_LEAKED_BYTES || HOSHI_TRACE_ALLOCATIONS
	#define WHEN_COUNT_OR_TRACE(...) __VA_ARGS__
#else
	#define WHEN_COUNT_OR_TRACE(...) ;
#endif

WHEN_COUNT(
	size_t hoshi_leakedBytes = 0;
);

WHEN_TRACE(
	static size_t hoshi_newAllocations = 0, hoshi_freedAllocations = 0;
);

void *hoshi_realloc(void *pointer, size_t oldSize, size_t newSize) {
	WHEN_COUNT(
		if (newSize > oldSize) {
			hoshi_leakedBytes += newSize - oldSize;
		} else if (newSize < oldSize) {
			hoshi_leakedBytes -= oldSize - newSize;
		}
	);

	WHEN_TRACE(
		printf(
			"hoshi_realloc: (%p) %zu -> %zu (leaked: %zu, news: %zu, frees: %zu) [change: ",
			pointer,
			oldSize,
			newSize,
			WHEN_COUNT(hoshi_leakedBytes,)
			hoshi_newAllocations,
			hoshi_freedAllocations
		);
		if (newSize > oldSize) {
			printf("+%zu", newSize - oldSize);
		} else if (newSize < oldSize) {
			printf("-%zu", oldSize - newSize);
		} else {
			printf("+0");
		}
		puts("]");
	);

	if (newSize == 0) {
		WHEN_TRACE(hoshi_freedAllocations++);
		free(pointer);
		return NULL;
	}

	WHEN_TRACE(
		if (oldSize == 0) {
			hoshi_newAllocations++;
		}
	)

	void *result = realloc(pointer, newSize);
	if (result == NULL) {
		exit(1); /* TODO: Throw a proper error message. */
	}
	return result;
}

#ifdef WHEN_COUNT
	#undef WHEN_COUNT
#endif
#ifdef WHEN_TRACE
	#undef WHEN_TRACE
#endif
#ifdef WHEN_COUNT_OR_TRACE
	#undef WHEN_COUNT_OR_TRACE
#endif

#endif
