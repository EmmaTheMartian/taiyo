#ifndef __HOSHI_VALUE_C__
#define __HOSHI_VALUE_C__

#include "value.h"
#include <stdio.h>

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
        }
}

#endif
