#include "binio.h"
#include <stdbool.h>
#include <stdio.h>

int main(void)
{
	FILE *w = fopen("test.bin", "wb");
	binio_writeBool(true, w);
	binio_writeI8(-127, w);
	binio_writeI16(-32767, w);
	binio_writeI32(1000000, w);
	binio_writeI64(-1000000, w);
	binio_writeU8(127, w);
	binio_writeU16(65000, w);
	binio_writeU32(1000000, w);
	binio_writeU64(1000000, w);
	binio_writeF32(123.45, w);
	binio_writeF64(12345.6789, w);
	fclose(w);

	FILE *r = fopen("test.bin", "rb");
	printf("%d\n", binio_readBool(r));
	printf("%d\n", binio_readI8(r));
	printf("%d\n", binio_readI16(r));
	printf("%d\n", binio_readI32(r));
	printf("%ld\n", binio_readI64(r));
	printf("%d\n", binio_readU8(r));
	printf("%d\n", binio_readU16(r));
	printf("%d\n", binio_readU32(r));
	printf("%ld\n", binio_readU64(r));
	printf("%g\n", binio_readF32(r));
	printf("%g\n", binio_readF64(r));
	fclose(r);
}
