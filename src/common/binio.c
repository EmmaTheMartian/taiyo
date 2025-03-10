#ifndef __TAIYO_COMMON_BINIO_C__
#define __TAIYO_COMMON_BINIO_C__

/*
 * binio.c (Binary I/O) contains utilties for reading and writing to binary files.
 * All operations read/write in big-endian.
 */

#include "../hoshi/value.h"
#include <stdint.h>
#include <stdio.h>

/* writers */

void binio_fputu8(uint8_t v, FILE *file)
{
	fputc(v, file);
}

void binio_fputu16(uint16_t v, FILE *file)
{
	fputc((v >> 8) & 0xFF, file);
	fputc(v & 0xFF, file);
}

void binio_fputu32(uint32_t v, FILE *file)
{
	fputc((v >> 24) & 0xFF, file);
	fputc((v >> 16) & 0xFF, file);
	fputc((v >> 8) & 0xFF, file);
	fputc(v & 0xFF, file);
}

void binio_fputd(double v, FILE *file)
{
	/* doubles are 8 bytes according to IEEE, which C follows.
	 * for now, I'm just going to trust that this will work cross-platform. */
	fwrite(&v, sizeof(double), 1, file);
}

void binio_fputValue(hoshi_Value value, FILE *file)
{
	fwrite(&value, sizeof(hoshi_Value), 1, file);
}

/* readers */

uint8_t binio_freadu8(FILE *file)
{
	uint8_t data;
	fread(&data, sizeof(uint8_t), 1, file);
	return data;
}

uint16_t binio_freadu16(FILE *file)
{
	return ((getc(file) << 8) & 0xFF) | (getc(file) & 0xFF);
}

uint32_t binio_freadu32(FILE *file)
{
	return
		((getc(file) << 24) & 0xFF) |
		((getc(file) << 16) & 0xFF) |
		((getc(file) << 8) & 0xFF) |
		(getc(file) & 0xFF);
}

double binio_freadd(FILE *file)
{
	/* doubles are 8 bytes according to IEEE, which C follows.
	 * for now, I'm just going to trust that this will work cross-platform. */
	double data;
	fread(&data, sizeof(double), 1, file);
	return data;
}

hoshi_Value binio_freadValue(FILE *file)
{
	hoshi_Value data;
	fread(&data, sizeof(hoshi_Value), 1, file);
	return data;
}

#endif
