#ifndef __TAIYO_COMMON_FILEIO_C__
#define __TAIYO_COMMON_FILEIO_C__

/*
 * fileio.c contains a handful of small file IO helpers.
 */

#include <stdio.h>
#include <stdlib.h>

/* Reads a file and mutates the provided fileSize to equal the size of the file in bytes. */
char *taiyoCommon_readFileWithSize(const char *filePath, size_t *fileSize)
{
	FILE *file = fopen(filePath, "rb");
	if (file == NULL) {
		fprintf(stderr, "error: could not open file: %s\n", filePath);
		exit(74);
	}

	fseek(file, 0L, SEEK_END);
	*fileSize = ftell(file);
	rewind(file);

	char *buffer = malloc(*fileSize + 1);
	if (buffer == NULL) {
		fprintf(stderr, "error: not enough memory to read `%s`", filePath);
		exit(74);
	}
	size_t bytesRead = fread(buffer, sizeof(char), *fileSize, file);
	if (bytesRead < *fileSize) {
		fprintf(stderr, "error: could not read file `%s`", filePath);
		exit(74);
	}
	buffer[bytesRead] = '\0';

	fclose(file);
	return buffer;
}

char *taiyoCommon_readFile(const char *filePath)
{
	size_t size;
	return taiyoCommon_readFileWithSize(filePath, &size);
}

#endif
