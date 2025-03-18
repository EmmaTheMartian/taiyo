#ifndef __HOSHI_VALUE_H__
#define __HOSHI_VALUE_H__

#include <stdbool.h>

#define HOSHI_NUMBER(value) ((hoshi_Value){ HOSHI_TYPE_NUMBER, { .number = value } })
#define HOSHI_BOOL(value) ((hoshi_Value){ HOSHI_TYPE_BOOL, { .boolean = value } })
#define HOSHI_NIL ((hoshi_Value){ HOSHI_TYPE_NIL, { .number = 0 } })
#define HOSHI_OBJECT(value) ((hoshi_Value){ HOSHI_TYPE_OBJECT, { .object = (hoshi_Object *)value } })

#define HOSHI_AS_NUMBER(value) ((value).as.number)
#define HOSHI_AS_BOOL(value) ((value).as.boolean)
#define HOSHI_AS_OBJECT(value) ((value).as.object)

#define HOSHI_IS_NUMBER(value) ((value).type == HOSHI_TYPE_NUMBER)
#define HOSHI_IS_BOOL(value) ((value).type == HOSHI_TYPE_BOOL)
#define HOSHI_IS_NIL(value) ((value).type == HOSHI_TYPE_NIL)
#define HOSHI_IS_OBJECT(value) ((value).type == HOSHI_TYPE_OBJECT)

typedef struct hoshi_Object hoshi_Object;
typedef struct hoshi_ObjectString hoshi_ObjectString;

typedef enum {
	HOSHI_TYPE_NUMBER,
	HOSHI_TYPE_BOOL,
	HOSHI_TYPE_NIL,
	HOSHI_TYPE_OBJECT,
} hoshi_ValueType;

typedef struct {
	hoshi_ValueType type;
	union {
		bool boolean;
		double number;
		hoshi_Object *object;
	} as;
} hoshi_Value;

void hoshi_printValue(hoshi_Value value);
bool hoshi_valuesEqual(hoshi_Value a, hoshi_Value b);

#endif
