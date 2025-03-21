#!/usr/bin/env sh

set -e

mkdir -p target

libhoshi_flags="-o target/libhoshi.so -fPIC -shared"
libhoshi_debug_flags="
	-DHOSHI_ENABLE_TRACE_EXECUTION_DEBUGGING=1
	-DHOSHI_DISASSEMBLER_ENABLE_RAW_CODE_DUMP=1
	-DHOSHI_DISASSEMBLER_ENABLE_CONSTANT_DUMP=1
	-DHOSHI_ENABLE_CHUNK_WRITE_DEBUG_INFO=1
	-DHOSHI_ENABLE_CHUNK_READ_DEBUG_INFO=1
	-DHOSHI_ENABLE_CHUNK_DEBUG_FLAGS=1"
libhoshi_prod_flags="-O3"
libhoshi_sources="
	src/hoshi/binio/binio.c
	src/hoshi/chunk_loader.c
	src/hoshi/chunk_writer.c
	src/hoshi/chunk.c
	src/hoshi/debug.c
	src/hoshi/memory.c
	src/hoshi/object.c
	src/hoshi/value.c
	src/hoshi/vm.c"

hoshi_flags="-o target/hoshi"
hoshi_debug_flags=$libhoshi_debug_flags
libhoshi_prod_flags=$libhoshi_prod_flags
hoshi_sources="
	src/hoshi/main.c
	target/libhoshi.so"

hir_flags="-o target/hir"
hir_debug_flags="
	-DHIR_ENABLE_PRINT_CODE=1
	-DHIR_ENABLE_PRINT_DISASSEMBLY=1
	-DHIR_ENABLE_TOKEN_DUMP=1"
hir_prod_flags="-O3"
hir_sources="
	src/hir/main.c
	src/hir/compiler.c
	src/hir/lexer.c
	src/common/thirdparty/asprintf.c
	./libhoshi.so"

cc () {
	echo "-> gcc $@"
	gcc $@
}

for arg in "$@"
do
	case "$arg" in
		"libhoshi"      ) cc "$libhoshi_flags $libhoshi_sources" ;;
		"libhoshi-debug") cc "$libhoshi_flags $libhoshi_debug_flags $libhoshi_sources" ;;
		"libhoshi-prod" ) cc "$libhoshi_flags $libhoshi_prod_flags $libhoshi_sources" ;;
		"hoshi"         ) cc "$hoshi_flags $hoshi_sources" ;;
		"hoshi-debug"   ) cc "$hoshi_flags $hoshi_debug_flags $hoshi_sources" ;;
		"hoshi-prod"    ) cc "$hoshi_flags $hoshi_prod_flags $hoshi_sources" ;;
		"hir"           ) cc "$hir_flags $hir_sources" ;;
		"hir-debug"     ) cc "$hir_flags $hir_debug_flags $hir_sources" ;;
		"hir-prod"      ) cc "$hir_flags $hir_prod_flags $hir_sources" ;;
		*               ) echo "Unknown command: $" ;;
	esac
done
