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

static int hoshi_decodeBytes(hoshi_Chunk *chunk, const char *source, size_t offset)
{
#define WRITE_AND_OFFSET() hoshi_writeChunk(chunk, source[offset], 1)
	/* first byte is assumed to be the instruction */
	hoshi_OpCode op = source[offset];
	hoshi_writeChunk(chunk, op, 1);

	/* some instructions have multiple arguments after them, which we need to decode too */
	switch (op) {
		case HOSHI_OP_CONSTANT:
			WRITE_AND_OFFSET();
			return 2;
		case HOSHI_OP_CONSTANT_LONG:
			WRITE_AND_OFFSET();
			WRITE_AND_OFFSET();
			WRITE_AND_OFFSET();
			return 3;
		default:
			return 1;
	}
#undef WRITE_AND_OFFSET
}

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

/* Writes the chunk to a C file which interprets the chunk. Weird, but it works. */
void hoshi_writeChunkToC(hoshi_Chunk *chunk, FILE *file)
{
	fprintf(file, "#include <hoshi/vm.h>\n#include <hoshi/chunk.h>\n\nstatic uint8_t bytecode[] = {\n");
	for (size_t i = 0; i < chunk->count; i++) {
		fprintf(file, "\t%d,\n", chunk->code[i]);
	}
	fprintf(file, "};\n\nstatic hoshi_Value constants[] = {\n");
	for (size_t i = 0; i < chunk->constants.count; i++) {
		//FIXME
		fprintf(file, "\t%g,\n", chunk->constants.values[i]);
	}
	fprintf(file, "};\n\nstatic hoshi_LineStart lines[] = {\n");
	for (size_t i = 0; i < chunk->lineCount; i++) {
		fprintf(file, "\t{ %d, %d },\n", chunk->lines[i].offset, chunk->lines[i].line);
	}
	fprintf(
		file,
		"};\n\n"
		"static hoshi_Chunk chunk = {\n"
			"\t%d,\n"
			"\t%d,\n"
			"\t&bytecode[0],\n"
			"\t{ %d, %d, &constants[0] },\n"
			"\t%d,\n"
			"\t%d,\n"
			"\t&lines[0],\n"
		"};\n\n"
		"int main(void)\n"
		"{\n"
			"\thoshi_VM vm;\n"
			"\thoshi_initVM(&vm);\n"
			"\thoshi_runChunk(&vm, &chunk);\n"
			"\thoshi_freeVM(&vm);\n"
			"\treturn 0;\n"
		"}\n",
		chunk->count,
		chunk->count,
		chunk->constants.count,
		chunk->constants.count,
		chunk->lineCount,
		chunk->lineCount
	);
}

/* Same as hoshi_writeChunkToC but minimized, i.e, non-human readable */
void hoshi_writeChunkToCNonHuman(hoshi_Chunk *chunk, FILE *file)
{
	fprintf(file, "#include<hoshi/vm.h>\n#include<hoshi/chunk.h>\nstatic uint8_t a[]={");
	for (size_t i = 0; i < chunk->count; i++) {
		fprintf(file, "%d,", chunk->code[i]);
	}
	fprintf(file, "};static hoshi_Value b[]={");
	for (size_t i = 0; i < chunk->constants.count; i++) {
		//FIXME
		fprintf(file, "%g,", chunk->constants.values[i]);
	}
	fprintf(file, "};static hoshi_LineStart c[]={");
	for (size_t i = 0; i < chunk->lineCount; i++) {
		fprintf(file, "{%d,%d},", chunk->lines[i].offset, chunk->lines[i].line);
	}
	fprintf(
		file,
		"};"
		"static hoshi_Chunk d={%d,%d,&a[0],{%d,%d,&b[0]},%d,%d,&c[0]};"
		"int main(void){"
			"hoshi_VM v;"
			"hoshi_initVM(&v);"
			"hoshi_runChunk(&v,&d);"
			"hoshi_freeVM(&v);"
			"return 0;"
		"}",
		chunk->count,
		chunk->count,
		chunk->constants.count,
		chunk->constants.count,
		chunk->lineCount,
		chunk->lineCount
	);
}

#undef WRITE_CHUNK_FLAG

#endif