#ifndef __HOSHI_VALUE_C__
#define __HOSHI_VALUE_C__

#include "value.h"
#include "object.h"
#include <stdio.h>
#include <string.h>

void hoshi_printValue(hoshi_Value value)
{
	switch (value.type) {
		case HOSHI_TYPE_NUMBER:
			printf("%g", HOSHI_AS_NUMBER(value));
			break;
		case HOSHI_TYPE_BOOL:
			printf(HOSHI_AS_BOOL(value) ? "true" : "false");
			break;
		case HOSHI_TYPE_NIL:
			printf("nil");
			break;
		case HOSHI_TYPE_OBJECT:
			hoshi_printObject(value);
			break;
        }
}

bool hoshi_valuesEqual(hoshi_Value a, hoshi_Value b)
{
	if (a.type != b.type) {
		return false;
	}
	switch (a.type) {
		case HOSHI_TYPE_NUMBER:
			return HOSHI_AS_NUMBER(a) == HOSHI_AS_NUMBER(b);
		case HOSHI_TYPE_NIL:
			return true;
		case HOSHI_TYPE_BOOL:
			return HOSHI_AS_BOOL(a) == HOSHI_AS_BOOL(b);
		case HOSHI_TYPE_OBJECT:
			return HOSHI_AS_OBJECT(a) == HOSHI_AS_OBJECT(b);
	}
	return false;
}

#endif
