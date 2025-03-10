#ifndef __HOSHI_CONFIG_H__
#define __HOSHI_CONFIG_H__

/* Runtime Configuration */

#ifndef HOSHI_ENABLE_TRACE_EXECUTION_DEBUGGING
	/* Set to `1` to enable MUCH more debugging information at runtime. */
	#define HOSHI_ENABLE_TRACE_EXECUTION_DEBUGGING 0
#endif

#ifndef HOSHI_STACK_SIZE
	#define HOSHI_STACK_SIZE 256
#endif

#ifndef HOSHI_COUNT_LEAKED_BYTES
	/* When true, the hoshi_realloc() function in memory.c will count bytes to ensure no memory is leaked.
	 * This does come with a very very minor performance toll, but being able to track down leaked memory is absolutely worth it. */
	#define HOSHI_COUNT_LEAKED_BYTES 1
#endif

/* Disassembler Configuration */

#ifndef HOSHI_DISASSEMBLER_ENABLE_RAW_CODE_DUMP
	/* Set to `1` to enable a raw dump of all bytecode instructions in the disassembler. */
	#define HOSHI_DISASSEMBLER_ENABLE_RAW_CODE_DUMP 0
#endif

#ifndef HOSHI_DISASSEMBLER_ENABLE_CONSTANT_DUMP
	/* Set to `1` to enable a raw dump of all constants in the disassembler. */
	#define HOSHI_DISASSEMBLER_ENABLE_CONSTANT_DUMP 1
#endif

#endif
