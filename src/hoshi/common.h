#ifndef __HOSHI_COMMON_H__
#define __HOSHI_COMMON_H__

#include <stdint.h>
#include <stdbool.h>

#define HOSHI_VERSION_MAJOR 1
#define HOSHI_VERSION_MINOR 0
#define HOSHI_VERSION_STRING "1.0"
#define HOSHI_VERSION ((hoshi_Version){ HOSHI_VERSION_MAJOR, HOSHI_VERSION_MINOR })

typedef struct {
	uint16_t major;
	uint16_t minor;
} hoshi_Version;

bool hoshi_versionMatches(hoshi_Version a, hoshi_Version b);
bool hoshi_versionNotMatches(hoshi_Version a, hoshi_Version b);
bool hoshi_versionNewerThan(hoshi_Version a, hoshi_Version b);
bool hoshi_versionOlderThan(hoshi_Version a, hoshi_Version b);
bool hoshi_versionNewerThanOrEquals(hoshi_Version a, hoshi_Version b);
bool hoshi_versionOlderThanOrEquals(hoshi_Version a, hoshi_Version b);

#endif
