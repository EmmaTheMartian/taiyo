#ifndef __HOSHI_HASH_TABLE_H__
#define __HOSHI_HASH_TABLE_H__

#include "value.h"
#include <stdint.h>

#define HOSHI_TABLE_MAX_LOAD 0.75

typedef struct hoshi_TableEntry {
	hoshi_ObjectString *key;
	hoshi_Value value;
	struct hoshi_TableEntry *next;
} hoshi_TableEntry;

typedef struct {
	int count;
	int capacity;
	hoshi_TableEntry *entries;
} hoshi_Table;

void hoshi_initTable(hoshi_Table *table);
void hoshi_freeTable(hoshi_Table *table);
hoshi_TableEntry *hoshi_tableFind(hoshi_Table *table, hoshi_ObjectString *key);
bool hoshi_tableSet(hoshi_Table *table, hoshi_ObjectString *key, hoshi_Value value);
bool hoshi_tableGet(hoshi_Table *table, hoshi_ObjectString *key, hoshi_Value *value);
bool hoshi_tableDelete(hoshi_Table *table, hoshi_ObjectString *key);
void hoshi_tableCopyAllFrom(hoshi_Table *from, hoshi_Table *to);
hoshi_ObjectString *hoshi_tableFindString(hoshi_Table *table, const char *chars, int length, uint64_t hash);

#endif
