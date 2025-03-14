#ifndef __HOSHI_OBJECT_C__
#define __HOSHI_OBJECT_C__

#include "object.h"
#include "memory.h"
#include "value.h"
#include <stdio.h>

hoshi_Object *hoshi_allocateObject(hoshi_ObjectTracker *tracker, size_t size, hoshi_ObjectType type)
{
	hoshi_Object *object = (hoshi_Object *)hoshi_realloc(NULL, 0, size);
	object->type = type;
	object->next = tracker->objects;
	tracker->objects = object;
	return object;
}

void hoshi_freeObject(hoshi_Object *object)
{
	switch (object->type) {
		case HOSHI_OBJTYPE_STRING: {
			hoshi_ObjectString *string = (hoshi_ObjectString *)object;
			HOSHI_FREE_ARRAY(char, string->chars, string->length);
			HOSHI_FREE(hoshi_ObjectString, string);
			break;
		}
	}
}

hoshi_ObjectString *hoshi_allocateString(hoshi_ObjectTracker *tracker, char *chars, int length)
{
	hoshi_ObjectString *string = HOSHI_ALLOCATE_OBJECT(tracker, hoshi_ObjectString, HOSHI_OBJTYPE_STRING);
	string->length = length;
	string->chars = chars;
	return string;
}

hoshi_ObjectString *hoshi_copyString(hoshi_ObjectTracker *tracker, const char *chars, int length)
{
	char *heapChars = HOSHI_ALLOCATE(char, length + 1);
	memcpy(heapChars, chars, length);
	heapChars[length] = '\0';
	return hoshi_allocateString(tracker, heapChars, length);
}

hoshi_ObjectString *hoshi_takeString(hoshi_ObjectTracker *tracker, char *chars, int length)
{
	return hoshi_allocateString(tracker, chars, length);
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
