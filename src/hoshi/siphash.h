#ifndef __HOSHI_SIPHASH_H__
#define __HOSHI_SIPHASH_H__

#include <stdint.h>

uint64_t siphash24(const void *src, unsigned long src_sz, const char key[16]);

#endif