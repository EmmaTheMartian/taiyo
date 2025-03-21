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

#ifndef HOSHI_LOCALS_SIZE
	#define HOSHI_LOCALS_SIZE 256
#endif

#ifndef HOSHI_MAX_SCOPE_DEPTH
	#define HOSHI_MAX_SCOPE_DEPTH 256
#endif

#ifndef HOSHI_ENABLE_GLOBAL_NAME_DUMP
	/* Set to `1` to enable a dump of all global names when the VM starts. */
	#define HOSHI_ENABLE_GLOBAL_NAME_DUMP 1
#endif

/* Memory configuration */

#ifndef HOSHI_COUNT_LEAKED_BYTES
	/* When true, the hoshi_realloc() function in memory.c will count bytes to ensure no memory is leaked.
	 * This does come with a very very minor performance toll, but being able to track down leaked memory is absolutely worth it.
	 * This is fairly inaccurate since it only counts calls with hoshi_realloc(), so for more proper memory leak testing, use memwatch and/or valgrind. */
	#define HOSHI_COUNT_LEAKED_BYTES 0
#endif

#ifndef HOSHI_TRACE_ALLOCATIONS
	/* When `1`, hoshi_realloc() will log all allocations. This will result in potentially a LOT of stdout, but helps in debugging memory leaks.
	 * If HOSHI_COUNT_LEAKED_BYTES is `1` as well, then leaked bytes are logged with allocations. */
	#define HOSHI_TRACE_ALLOCATIONS 0
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

/* Chunk read/write debugging */

#ifndef HOSHI_ENABLE_CHUNK_WRITE_DEBUG_INFO
	/* Set to `1` to enable debug information when writing chunks. */
	#define HOSHI_ENABLE_CHUNK_WRITE_DEBUG_INFO 0
#endif

#ifndef HOSHI_ENABLE_CHUNK_READ_DEBUG_INFO
	/* Set to `1` to enable debug information when reading chunks. */
	#define HOSHI_ENABLE_CHUNK_READ_DEBUG_INFO 0
#endif

#ifndef HOSHI_ENABLE_CHUNK_DEBUG_FLAGS
	/* Set to `1` to enable flags in written Hoshi files (i.e, `.code` section to mark instructions or `.consts` for constants, etc etc)
	 * Note that files compiled with debug flags cannot be parsed unless this flag is also set on reader. */
	#define HOSHI_ENABLE_CHUNK_DEBUG_FLAGS 0
#endif

/* main.c config */

#ifndef HOSHI_ENABLE_NOP_MODE
	#define HOSHI_ENABLE_NOP_MODE 0
#endif

#endif
