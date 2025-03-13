#ifndef __HOSHI_OBJECT_H__
#define __HOSHI_OBJECT_H__

#include "value.h"
#include "memory.h"
#include <string.h>

#define HOSHI_TYPEOF_OBJECT(value) (HOSHI_AS_OBJECT(value)->type)

#define HOSHI_IS_STRING(value) hoshi_isObjectType(value, HOSHI_OBJTYPE_STRING)

#define HOSHI_AS_STRING(value) ((hoshi_ObjectString *)HOSHI_AS_OBJECT(value))
#define HOSHI_AS_CSTRING(value) (((hoshi_ObjectString *)HOSHI_AS_OBJECT(value))->chars)

#define HOSHI_ALLOCATE_OBJECT(type, objectType) (type *)hoshi_allocateObject(sizeof(type), objectType);

typedef enum {
	HOSHI_OBJTYPE_STRING,
} hoshi_ObjectType;

struct hoshi_Object {
	hoshi_ObjectType type;
};

struct hoshi_ObjectString {
	hoshi_Object object;
	int length; /* TODO: LEB128 */
	char *chars;
};

/* Allocates an object of the given size and type. */
hoshi_Object *hoshi_allocateObject(size_t size, hoshi_ObjectType type);

/* Internal helper function to allocate a string of the given size.
 * Note: For most use cases, you should probably use hoshi_copyString or hoshi_takeString. */
hoshi_ObjectString *hoshi_allocateString(char *chars, int length);

/* Helper function to copy the given string and allocate a string object. */
hoshi_ObjectString *hoshi_copyString(const char *chars, int length);

/* Helper function to allocate a string object, assuming ownership of the provided string.
 * If you're unsure of ownership, just use hoshi_copyString to be sure. */
hoshi_ObjectString *hoshi_takeString(char *chars, int length);

/* Prints an object value. */
void hoshi_printObject(hoshi_Value value);

static inline bool hoshi_isObjectType(hoshi_Value value, hoshi_ObjectType type)
{
	return HOSHI_IS_OBJECT(value) && HOSHI_AS_OBJECT(value)->type == type;
}

#endif
