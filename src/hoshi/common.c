#ifndef __HOSHI_COMMON_C__
#define __HOSHI_COMMON_C__

#include "common.h"
#include <stdbool.h>

#if MEMWATCH
#include "memwatch.h"
#endif

#define HOSHI_CHECK_FUNC(name, op) \
	bool name (hoshi_Version a, hoshi_Version b) \
	{ \
		return (a.major op b.major) && (a.minor op b.minor); \
	}

HOSHI_CHECK_FUNC(hoshi_versionMatches, ==)
HOSHI_CHECK_FUNC(hoshi_versionNotMatches, !=)
HOSHI_CHECK_FUNC(hoshi_versionNewerThan, >)
HOSHI_CHECK_FUNC(hoshi_versionOlderThan, <)
HOSHI_CHECK_FUNC(hoshi_versionNewerThanOrEquals, >=)
HOSHI_CHECK_FUNC(hoshi_versionOlderThanOrEquals, <=)

#undef HOSHI_CHECK_FUNC

#endif
