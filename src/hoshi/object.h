#ifndef __HOSHI_OBJECT_H__
#define __HOSHI_OBJECT_H__

#include "value.h"
#include "memory.h"
#include "vm.h"
#include <string.h>

#define HOSHI_TYPEOF_OBJECT(value) (HOSHI_AS_OBJECT(value)->type)

#define HOSHI_IS_STRING(value) hoshi_isObjectType(value, HOSHI_OBJTYPE_STRING)

#define HOSHI_AS_STRING(value) ((hoshi_ObjectString *)HOSHI_AS_OBJECT(value))
#define HOSHI_AS_CSTRING(value) (((hoshi_ObjectString *)HOSHI_AS_OBJECT(value))->chars)

#define HOSHI_ALLOCATE_OBJECT(tracker, type, objectType) (type *)hoshi_allocateObject(tracker, sizeof(type), objectType);

typedef enum {
	HOSHI_OBJTYPE_STRING,
} hoshi_ObjectType;

struct hoshi_Object {
	hoshi_ObjectType type;
	hoshi_Object *next; /* Pointer to the next object */
};

struct hoshi_ObjectString {
	hoshi_Object object;
	bool ownsChars;
	int length; /* TODO: LEB128 */
	const char *chars;
};

/* Allocates an object of the given size and type. */
hoshi_Object *hoshi_allocateObject(hoshi_ObjectTracker *tracker, size_t size, hoshi_ObjectType type);

/* Frees an object, you typically do not need to call this manually and can leave it to the VM to clean up itself. */
void hoshi_freeObject(hoshi_Object *object);

/* Helper function to allocate a string with the given characters and length.
 * Set `ownsChars` to true when the characters are owned by the object, and false when they are heap allocated and not owned by the object. */
hoshi_ObjectString *hoshi_makeString(hoshi_ObjectTracker *tracker, bool ownsChars, char *chars, int length);

/* Prints an object value. */
void hoshi_printObject(hoshi_Value value);

static inline bool hoshi_isObjectType(hoshi_Value value, hoshi_ObjectType type)
{
	return HOSHI_IS_OBJECT(value) && HOSHI_AS_OBJECT(value)->type == type;
}

#endif
