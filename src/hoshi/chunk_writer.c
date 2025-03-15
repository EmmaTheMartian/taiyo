#ifndef __HOSHI_CHUNK_WRITER_C__
#define __HOSHI_CHUNK_WRITER_C__

#include "chunk.h"
#include "object.h"
#include "common.h"
#include "config.h"
#include "../binio/binio.h"

/* These macros get #undef'd at the end of the file */
#if HOSHI_ENABLE_CHUNK_DEBUG_FLAGS
	#define WRITE_CHUNK_FLAG(flag, file) fputs(flag, file)
#else
	#define WRITE_CHUNK_FLAG(flag, file) ;
#endif

void hoshi_writeObjectToFile(hoshi_Object *object, FILE *file)
{
	/* Write type */
	WRITE_CHUNK_FLAG("/", file);
	binio_writeU8(object->type, file);

	/* Write data */
	WRITE_CHUNK_FLAG("=", file);
	switch (object->type) {
		case HOSHI_OBJTYPE_STRING: {
			hoshi_ObjectString *string = (hoshi_ObjectString *)object;
			binio_writeU32(string->length, file); /* TODO: LEB128 */
			fwrite(string->chars, sizeof(char), string->length, file);
			break;
		}
	}
}

void hoshi_writeValueToFile(hoshi_Value *value, FILE *file)
{
	/* Write the type identifier */
	WRITE_CHUNK_FLAG("#", file);
	binio_writeU8(value->type, file);

	/* Write data */
	WRITE_CHUNK_FLAG("=", file);
	switch (value->type) {
		case HOSHI_TYPE_NUMBER:
			binio_writeF64(value->as.number, file);
			break;
		case HOSHI_TYPE_BOOL:
			binio_writeU8(value->as.boolean, file);
			break;
		case HOSHI_TYPE_NIL:
			/* nop */
			break;
		case HOSHI_TYPE_OBJECT:
			hoshi_writeObjectToFile(value->as.object, file);
			break;
	}
}

void hoshi_writeChunkToFile(hoshi_Chunk *chunk, FILE *file)
{
#if HOSHI_ENABLE_CHUNK_WRITE_DEBUG_INFO
	#define DBG(format, ...) printf("[CHUNK_WRITE_DEBUG_INFO] (offset: %06zu bytes) " format , ftell(file) __VA_OPT__(,) __VA_ARGS__)
#else
	#define DBG(...) ;
#endif

	/* magic number */
	WRITE_CHUNK_FLAG(".magic", file);
	fwrite(hoshi_magicNumber, 7, 1, file);
	DBG("Wrote magic number\n");

	/* version */
	WRITE_CHUNK_FLAG(".version", file);
	binio_writeU16(HOSHI_VERSION_MAJOR, file);
	binio_writeU16(HOSHI_VERSION_MINOR, file);
	DBG("Wrote version info (%d.%d)\n", HOSHI_VERSION_MAJOR, HOSHI_VERSION_MINOR);

	/* constant pool */
	WRITE_CHUNK_FLAG(".consts", file);
	binio_writeU16(chunk->constants.count, file);
	DBG("Wrote constant pool count (%d)\n", chunk->constants.count);
	for (size_t i = 0; i < chunk->constants.count; i++) {
		DBG("  | Writing constant %zu: ", i);
		#if HOSHI_ENABLE_CHUNK_WRITE_DEBUG_INFO
			hoshi_printValue(chunk->constants.values[i]);
			puts("");
		#endif

		hoshi_writeValueToFile(&chunk->constants.values[i], file);
	}
	DBG("Wrote constant pool\n");

	/* instructions */
	WRITE_CHUNK_FLAG(".code", file);
	binio_writeU32(chunk->count, file);
	DBG("Wrote instruction count (%d)\n", chunk->count);
	for (size_t i = 0; i < chunk->count; i++) {
		binio_writeU8(chunk->code[i], file);
	}
	DBG("Wrote instructions\n");

	/* line markers */
	WRITE_CHUNK_FLAG(".lines", file);
	binio_writeU32(chunk->lineCount, file);
	DBG("Wrote line count (%d)\n", chunk->lineCount);
	for (size_t i = 0; i < chunk->lineCount; i++) {
		binio_writeU32(chunk->lines[i].offset, file);
		binio_writeU32(chunk->lines[i].line, file);
		DBG("  | Wrote line marker %zu: O:%d L:%d\n", i, chunk->lines[i].offset, chunk->lines[i].line);
	}
	DBG("Wrote line markers\n");

	WRITE_CHUNK_FLAG(".notes", file);

	DBG("Wrote chunk\n");
#undef DBG
}

#undef WRITE_CHUNK_FLAG

#endif