#ifndef __HIR_CONFIG_H__
#define __HIR_CONFIG_H__

/* Compiler Configuration */

#ifndef HIR_ENABLE_PRINT_CODE
	/* Set to `1` to enable debugging information as the compiler runs. */
	#define HIR_ENABLE_PRINT_CODE 0
#endif

#ifndef HIR_ENABLE_PRINT_DISASSEMBLY
	/* Set to `1` to print a disassembly after compilation. */
	#define HIR_ENABLE_PRINT_DISASSEMBLY 0
#endif

#ifndef HIR_ENABLE_TOKEN_DUMP
	/* Set to `1` to print a token dump before compilation. */
	#define HIR_ENABLE_TOKEN_DUMP 0
#endif

#endif
