#ifndef __BINIO_C__
#define __BINIO_C__

#include "binio.h" /* function declarations are defined by the BINIO_FUNC() macro. */

#define BINIO_FUNC(name, type) \
	void binio_write##name(type v, FILE *file) { \
		fwrite(&v, sizeof(v), 1, file); \
	} \
	type binio_read##name(FILE *file) { \
		type v; \
		fread(&v, sizeof(v), 1, file); \
		return v; \
	}

BINIO_FUNC(Bool, bool);

BINIO_FUNC(I8, int8_t);
BINIO_FUNC(I16, int16_t);
BINIO_FUNC(I32, int32_t);
BINIO_FUNC(I64, int64_t);

BINIO_FUNC(U8, uint8_t);
BINIO_FUNC(U16, uint16_t);
BINIO_FUNC(U32, uint32_t);
BINIO_FUNC(U64, uint64_t);

BINIO_FUNC(F32, float);
BINIO_FUNC(F64, double);

BINIO_FUNC(String, char *);

#undef BINIO_FUNC

#endif
