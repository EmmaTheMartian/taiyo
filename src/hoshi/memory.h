#ifndef __HOSHI_MEMORY_H__
#define __HOSHI_MEMORY_H__

#include <stddef.h>

/* Allocator macro */
#define HOSHI_ALLOCATE(type, count) (type *)hoshi_realloc(NULL, 0, sizeof(type) * count)

/* Frees the given object. */
#define HOSHI_FREE(type, pointer) hoshi_realloc(pointer, sizeof(type), 0)

/* Doubles the given capacity, used in dynamic arrays.
 * The growth factor can be modified by changing the `2` here. */
#define HOSHI_GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity) * 2)

/* Grows the array to `sizeof(type) * newCount` from `sizeof(type) * oldCount`. */
#define HOSHI_GROW_ARRAY(type, pointer, oldCount, newCount) \
	(type *)hoshi_realloc(pointer, sizeof(type) * (oldCount), sizeof(type) * (newCount))

/* Frees the given array. */
#define HOSHI_FREE_ARRAY(type, pointer, oldCount) \
	(void)(hoshi_realloc(pointer, sizeof(type) * (oldCount), 0))

typedef struct hoshi_Object hoshi_Object;

/* Big brother is watching. */
typedef struct {
	hoshi_Object *objects; /* Linked list of allocated objects */
} hoshi_ObjectTracker;

void *hoshi_realloc(void *pointer, size_t oldSize, size_t newSize);

#endif
