#ifndef __HOSHI_OBJECT_C__
#define __HOSHI_OBJECT_C__

#include "object.h"
#include "hash_table.h"
#include "memory.h"
#include "value.h"
#include "vm.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* YOU ARE A BOZO!! */
/*static char hoshi_siphashKey[16] = {
	'Y', 'O', 'U', ' ',
	'A', 'R', 'E', ' ',
	'A', ' ',
	'B', 'O', 'Z', 'O',
	'!', '!'
};*/

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

/* TODO: Switch to SipHash. Current implementation is FNV-1a */
static uint32_t hoshi_hashString(const char *key, int length)
{
	uint32_t hash = 2166136261u;
	for (int i = 0; i < length; i++) {
		hash ^= (uint8_t)key[i];
		hash *= 16777619;
	}
	return hash;
}

hoshi_ObjectString *hoshi_makeString(hoshi_VM *vm, bool ownsString, char *chars, int length)
{
	uint64_t hash = hoshi_hashString(chars, length);

	hoshi_ObjectString *interned = hoshi_tableFindString(&vm->strings, chars, length, hash);
	if (interned != NULL) {
		if (ownsString) {
			HOSHI_FREE_ARRAY(char, chars, length + 1);
		}
		return interned;
	}

	hoshi_ObjectString *string = HOSHI_ALLOCATE_OBJECT(&vm->tracker, hoshi_ObjectString, HOSHI_OBJTYPE_STRING);
	string->ownsChars = ownsString;
	string->length = length;
	string->chars = chars;
	// string->hash = siphash24(chars, length, hoshi_siphashKey);
	string->hash = hash;
	hoshi_tableSet(&vm->strings, string, HOSHI_NIL);
	return string;
}

char *hoshi_formatString(hoshi_VM *vm, const char *string, int length)
{
	int formattedSize = length;
	char *formatted = HOSHI_ALLOCATE(char, formattedSize);

	char ch, prev = '\0';
	int len = length;
	int offset = 0;
	int i = 0;
	for (;;) {
		if (i >= len) {
			break;
		}

		ch = string[i];

		if (prev == '\\') {
			switch (ch) {
				case 'a': formatted[i-1] = '\a'; break;
				case 'b': formatted[i-1] = '\n'; break;
				case 'e': formatted[i-1] = '\e'; break;
				case 'f': formatted[i-1] = '\f'; break;
				case 'n': formatted[i-1] = '\n'; break;
				case 'r': formatted[i-1] = '\r'; break;
				case 't': formatted[i-1] = '\t'; break;
				case 'v': formatted[i-1] = '\v'; break;
				case '\\': formatted[i-1] = '\\'; break;
				case '\'': formatted[i-1] = '\''; break;
				case '\"': formatted[i-1] = '\"'; break;
				case '\?': formatted[i-1] = '\?'; break;
				default:
					hoshi_panic(vm, "invalid escape sequence: `\\%c`", ch);
					break;
			}
			formattedSize--;
			offset--;
			i++;
			prev = '\n';
			continue;
		} else {
			formatted[i + offset] = ch;
		}

		prev = ch;
		i++;
	}

	formatted[formattedSize] = '\0';

	if (formattedSize != len) {
		formatted = hoshi_realloc(formatted, sizeof(char) * len, sizeof(char) * formattedSize);
	}

	return formatted;
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
