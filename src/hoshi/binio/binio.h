#ifndef __BINIO_H__
#define __BINIO_H__

/*
 * binio.c (Binary I/O) contains utilties for reading and writing to binary files.
 * All operations read/write in big-endian.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

void binio_writeBool(bool v, FILE *file);
bool binio_readBool(FILE *file);

void binio_writeU8(uint8_t v, FILE *file);
uint8_t binio_readU8(FILE *file);

void binio_writeU16(uint16_t c, FILE *file);
uint16_t binio_readU16(FILE *file);

void binio_writeU32(uint32_t c, FILE *file);
uint32_t binio_readU32(FILE *file);

void binio_writeU64(uint64_t c, FILE *file);
uint64_t binio_readU64(FILE *file);

void binio_writeI8(int8_t v, FILE *file);
int8_t binio_readI8(FILE *file);

void binio_writeI16(int16_t v, FILE *file);
int16_t binio_readI16(FILE *file);

void binio_writeI32(int32_t v, FILE *file);
int32_t binio_readI32(FILE *file);

void binio_writeI64(int64_t v, FILE *file);
int64_t binio_readI64(FILE *file);

void binio_writeF32(float v, FILE *file);
float binio_readF32(FILE *file);

void binio_writeF64(double v, FILE *file);
double binio_readF64(FILE *file);

#endif
