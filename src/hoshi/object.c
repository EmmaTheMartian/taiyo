#ifndef __HOSHI_OBJECT_C__
#define __HOSHI_OBJECT_C__

#include "object.h"
#include "memory.h"
#include "value.h"
#include <stdio.h>
#include <string.h>

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
			if (string->ownsChars) {
				HOSHI_FREE_ARRAY(char, (char *)string->chars, string->length + 1);
			}
			HOSHI_FREE(hoshi_ObjectString, string);
			break;
		}
	}
}

hoshi_ObjectString *hoshi_makeString(hoshi_ObjectTracker *tracker, bool ownsString, char *chars, int length)
{
	hoshi_ObjectString *string = HOSHI_ALLOCATE_OBJECT(tracker, hoshi_ObjectString, HOSHI_OBJTYPE_STRING);
	string->ownsChars = ownsString;
	string->length = length;
	string->chars = chars;
	return string;
}

void hoshi_printObject(hoshi_Value value)
{
	switch (HOSHI_TYPEOF_OBJECT(value)) {
		case HOSHI_OBJTYPE_STRING:
			printf("%.*s", HOSHI_AS_STRING(value)->length, HOSHI_AS_CSTRING(value));
			break;
	}
}

#endif
