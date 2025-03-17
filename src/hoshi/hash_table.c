#ifndef __HOSHI_HASH_TABLE_C__
#define __HOSHI_HASH_TABLE_C__

#include "hash_table.h"
#include "memory.h"
#include "value.h"
#include "object.h"
#include <cstdint>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void hoshi_initTable(hoshi_Table *table)
{
	table->count = 0;
	table->capacity = 0;
	table->entries = NULL;
}

void hoshi_freeTable(hoshi_Table *table)
{
	HOSHI_FREE_ARRAY(hoshi_TableEntry, table->entries, table->capacity);
	hoshi_initTable(table);
}

hoshi_TableEntry *hoshi_tableFind(hoshi_Table *table, hoshi_ObjectString *key)
{
	uint32_t index = key->hash % table->capacity;
	hoshi_TableEntry *tombstone;
	for (;;) {
		hoshi_TableEntry *entry = &table->entries[index];
		if (entry->key == NULL) {
			if (HOSHI_IS_NIL(entry->value)) {
				/* Empty entry */
				return tombstone != NULL ? tombstone : entry;
			} else {
				/* We found a tombstone */
				if (tombstone == NULL) {
					tombstone = entry;
				}
			}
		} else if (entry->key == key) {
			/* We found the key */
			return entry;
		}
		index = (index + 1) % table->capacity;
	}
	return NULL;
}

void hoshi_tableAdjustCapacity(hoshi_Table *table, int capacity)
{
	hoshi_TableEntry *entries = HOSHI_ALLOCATE(hoshi_TableEntry, capacity);
	for (int i = 0; i < capacity; i++) {
		entries[i].key = NULL;
		entries[i].value = HOSHI_NIL();
	}

	/* Reinsert old elements */
	table->count = 0;
	for (int i = 0; i < table->capacity; i++) {
		hoshi_TableEntry *entry = &table->entries[i];
		if (entry->key == NULL) {
			continue;
		}

		hoshi_TableEntry *dest = hoshi_tableFind(table, entry->key);
		dest->key = entry->key;
		dest->value = entry->value;
		table->count++;
	}

	/* Free old entry array */
	HOSHI_FREE_ARRAY(hoshi_TableEntry, table->entries, table->capacity);

	/* Update table */
	table->entries = entries;
	table->capacity = capacity;
}

bool hoshi_tableSet(hoshi_Table *table, hoshi_ObjectString *key, hoshi_Value value)
{
	if (table->count + 1 > table->capacity * HOSHI_TABLE_MAX_LOAD) {
		int capacity = HOSHI_GROW_CAPACITY(table->capacity);
		hoshi_tableAdjustCapacity(table, capacity);
	}

	hoshi_TableEntry *entry = hoshi_tableFind(table, key);
	bool isNewKey = entry->key == NULL;
	if (isNewKey && HOSHI_IS_NIL(entry->value)) {
		table->count++;
	}
	entry->key = key;
	entry->value = value;
	return isNewKey;
}

bool hoshi_tableGet(hoshi_Table *table, hoshi_ObjectString *key, hoshi_Value *value)
{
	if (table->count == 0) {
		return false;
	}

	hoshi_TableEntry *entry = hoshi_tableFind(table, key);
	if (entry->key == NULL) {
		return false;
	}

	*value = entry->value;
	return true;
}

bool hoshi_tableDelete(hoshi_Table *table, hoshi_ObjectString *key)
{
	if (table->count == 0) {
		return false;
	}

	hoshi_TableEntry *entry = hoshi_tableFind(table, key);
	if (entry->key == NULL) {
		return false;
	}

	/* Place a tombstone */
	entry->key = NULL;
	entry->value = HOSHI_BOOL(true);

	return true;
}

void hoshi_tableCopyAllFrom(hoshi_Table *from, hoshi_Table *to)
{
	for (int i = 0; i < from->capacity; i++) {
		hoshi_TableEntry *entry = &from->entries[i];
		if (entry->key != NULL) {
			hoshi_tableSet(to, entry->key, entry->value);
		}
	}
}

hoshi_ObjectString *hoshi_tableFindString(hoshi_Table *table, const char *chars, int length, uint64_t hash)
{
	if (table->count == 0) {
		return false;
	}

	uint32_t index = hash % table->capacity;
	for (;;) {
		hoshi_TableEntry *entry = &table->entries[index];
		if (entry->key == NULL) {
			/* stop if we find an empty non-tombstone entry */
			if (HOSHI_IS_NIL(entry->value)) {
				return NULL;
			}
		} else if (entry->key->length == length && entry->key->hash == hash && memcmp(entry->key->chars, chars, length)) {
			/* found it! */
			return entry->key;
		}

		index = (index + 1) % table->capacity;
	}
}

#endif
