#ifndef __HOSHI_OBJECT_C__
#define __HOSHI_OBJECT_C__

#include "object.h"
#include "value.h"
#include <stdio.h>

hoshi_Object *hoshi_allocateObject(size_t size, hoshi_ObjectType type)
{
	hoshi_Object *object = (hoshi_Object *)hoshi_realloc(NULL, 0, size);
	object->type = type;
	return object;
}

hoshi_ObjectString *hoshi_allocateString(char *chars, int length)
{
	hoshi_ObjectString *string = HOSHI_ALLOCATE_OBJECT(hoshi_ObjectString, HOSHI_OBJTYPE_STRING);
	string->length = length;
	string->chars = chars;
	return string;
}

hoshi_ObjectString *hoshi_copyString(const char *chars, int length)
{
	char *heapChars = HOSHI_ALLOCATE(char, length + 1);
	memcpy(heapChars, chars, length);
	heapChars[length] = '\0';
	return hoshi_allocateString(heapChars, length);
}

hoshi_ObjectString *hoshi_takeString(char *chars, int length)
{
	return hoshi_allocateString(chars, length);
}

void hoshi_printObject(hoshi_Value value)
{
	switch (HOSHI_TYPEOF_OBJECT(value)) {
		case HOSHI_OBJTYPE_STRING:
			printf("%s", HOSHI_AS_CSTRING(value));
			break;
	}
}

#endif
