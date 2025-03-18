#ifndef __HOSHI_CHUNK_LOADER_H__
#define __HOSHI_CHUNK_LOADER_H__

#include "binio/binio.h"
#include "common.h"
#include "chunk_loader.h"
#include "chunk.h"
#include "memory.h"
#include "object.h"
#include "config.h"
#include "value.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* These macros get #undef'd at the end of the file */
#if HOSHI_ENABLE_CHUNK_DEBUG_FLAGS
	/* I use a string here so that the READ_CHUNK_FLAG calls make which flag they intend to read more obvious */
	#define READ_CHUNK_FLAG(flag, file) fseek(file, strlen(flag), SEEK_CUR)
#else
	#define READ_CHUNK_FLAG(len, file) ;
#endif

hoshi_Object *hoshi_readObjectFromFile(hoshi_VM *vm, FILE *file)
{
	/* Read object type */
	READ_CHUNK_FLAG("/", file);
	hoshi_ObjectType type = binio_readU8(file);

	/* Read object data */
	READ_CHUNK_FLAG("=", file);
	switch (type) {
		case HOSHI_OBJTYPE_STRING: {
			size_t length = binio_readU32(file);
			char *chars = HOSHI_ALLOCATE(char, length + 1);
			fread(chars, sizeof(char), length, file);
			chars[length] = '\0';
			return (hoshi_Object *)hoshi_makeString(vm, true, chars, length);
		}
	}
	fprintf(stderr, "internal error: hoshi_readObjectFromFile got a value of an unknown type: %d", type);
	return NULL;
}

hoshi_Value hoshi_readValueFromFile(hoshi_VM *vm, FILE *file)
{
	/* Read the type identifier */
	READ_CHUNK_FLAG("#", file);
	hoshi_ValueType type = binio_readU8(file);

	/* Read data */
	READ_CHUNK_FLAG("=", file);
	switch (type) {
		case HOSHI_TYPE_NUMBER: {
			double value = binio_readF64(file);
			return HOSHI_NUMBER(value);
		}
		case HOSHI_TYPE_BOOL: {
			bool value = binio_readU8(file);
			return HOSHI_BOOL(value);
		}
		case HOSHI_TYPE_NIL:
			return HOSHI_NIL;
		case HOSHI_TYPE_OBJECT: {
			hoshi_Object *object = hoshi_readObjectFromFile(vm, file);
			return HOSHI_OBJECT(object);
		}
	}
	fprintf(stderr, "internal error: hoshi_readValueFromFile got a value of an unknown type: %d", type);
	return HOSHI_NIL;
}

bool hoshi_readChunkFromFile(hoshi_VM *vm, hoshi_Chunk *chunk, FILE *file, hoshi_Version expectedVersion)
{
	hoshi_initChunk(chunk);

#if HOSHI_ENABLE_CHUNK_READ_DEBUG_INFO
	#define DBG(format, ...) printf("[CHUNK_READ_DEBUG_INFO] (offset: %06zu bytes) " format , ftell(file) __VA_OPT__(,) __VA_ARGS__)
#else
	#define DBG(...) ;
#endif

	/* I scope everything here so that local variables go out of their scope to keep the as small as possible while reading. */

	/* verify magic number */
	{
		DBG("Reading magic number\n");
		READ_CHUNK_FLAG(".magic", file);
		char magic[7] = {
			binio_readU8(file),
			binio_readU8(file),
			binio_readU8(file),
			binio_readU8(file),
			binio_readU8(file),
			binio_readU8(file),
			binio_readU8(file),
		};

#if HOSHI_ENABLE_CHUNK_READ_DEBUG_INFO
		for (int i = 0; i < 7; i++) {
			DBG("  (PREVIEW) %d, %d, %d\n", i, hoshi_magicNumber[i], magic[i]);
		}
#endif

		for (int i = 0; i < 7; i++) {
			DBG("  Magic number: (i: %d, expected: %d, got: %d)\n", i, hoshi_magicNumber[i], magic[i]);
			if (hoshi_magicNumber[i] != magic[i]) {
				fprintf(stderr, "error: failed to read chunk: magic number is invalid\n");
				return false;
			}
		}
	}

	/* verify version */
	{
		DBG("Reading version\n");
		READ_CHUNK_FLAG(".version", file);
		uint16_t major = binio_readU16(file);
		uint16_t minor = binio_readU16(file);
		DBG("Version: %d.%d\n", major, minor);
		hoshi_Version version = { major, minor };
		if (hoshi_versionOlderThan(version, expectedVersion)) {
			fprintf(stderr, "error: failed to read chunk: file version older than minimum readable version (got %d.%d, expected at least %d.%d)\n", major, minor, expectedVersion.major, expectedVersion.minor);
			return false;
		}
	}

	/* read constants */
	{
		DBG("Reading constant count\n");
		READ_CHUNK_FLAG(".consts", file);
		uint16_t constantCount = binio_readU16(file);
		DBG("Constant count: %d\n", constantCount);
		/* grow constant pool if needed */
		if (chunk->constants.capacity < constantCount + 1) {
			DBG("Growing constant pool\n");
			int oldCapacity = chunk->constants.capacity;
			chunk->constants.capacity = constantCount;
			chunk->constants.values = HOSHI_GROW_ARRAY(hoshi_Value, chunk->constants.values, oldCapacity, chunk->constants.capacity);
		}
		chunk->constants.count = constantCount;
		#if HOSHI_ENABLE_CHUNK_READ_DEBUG_INFO
		for (size_t i = 0; i < constantCount; i++) {
			hoshi_printValue(chunk->constants.values[i]);
			puts("");
		}
		#endif
		DBG("Reading constants\n");
		for (size_t i = 0; i < constantCount; i++) {
			hoshi_Value value = hoshi_readValueFromFile(vm, file);
			chunk->constants.values[i] = value;
#if HOSHI_ENABLE_CHUNK_READ_DEBUG_INFO
			printf("  | Read constant %zu: ", i);
			hoshi_printValue(chunk->constants.values[i]);
			puts("");
#endif
		}
	}

	/* read instructions */
	{
		DBG("Reading instruction count\n");
		READ_CHUNK_FLAG(".code", file);
		uint16_t instructionCount = binio_readU32(file);
		DBG("Instruction count: %d\n", instructionCount);
		/* grow instruction array if needed */
		if (chunk->capacity < instructionCount + 1) {
			DBG("Growing instruction array\n");
			int oldCapacity = chunk->capacity;
			chunk->capacity = instructionCount;
			chunk->code = HOSHI_GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
		}
		chunk->count = instructionCount;
		DBG("Reading instructions\n");
		for (size_t i = 0; i < instructionCount; i++) {
			chunk->code[i] = binio_readU8(file);
		}
	}

	/* read line markers */
	{
		DBG("Reading line count\n");
		READ_CHUNK_FLAG(".lines", file);
		uint16_t lineCount = binio_readU32(file);
		DBG("Line count: %d\n", lineCount);
		/* grow line marker capacity if needed */
		if (chunk->lineCapacity < lineCount + 1) {
			DBG("Growing line marker array\n");
			int oldCapacity = chunk->lineCapacity;
			chunk->lineCapacity = lineCount;
			chunk->lines = HOSHI_GROW_ARRAY(hoshi_LineStart, chunk->lines, oldCapacity, chunk->lineCapacity);
		}
		chunk->lineCount = lineCount;
		DBG("Reading line markers\n");
		for (size_t i = 0; i < lineCount; i++) {
			chunk->lines[i].offset = binio_readU16(file);
			chunk->lines[i].line = binio_readU16(file);
			DBG("  | Read line marker %zu: O:%d L:%d\n", i, chunk->lines[i].offset, chunk->lines[i].line);
		}
	}

	READ_CHUNK_FLAG(".notes", file);

	DBG("Read chunk\n");
	return true;
#undef DBG
}

#undef READ_CHUNK_FLAG

#endif