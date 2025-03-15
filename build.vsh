#!/usr/bin/env -S v run

import build
import arrays

// Utilities //
enum BuildMode {
	debug
	default
	release
}

struct Config {
pub mut:
	cc   string    = 'gcc'
	mode BuildMode = .default
	opts []string = ['-Wall']
}

struct Module {
pub:
	name       string
	sources    []string
	depends    []string
	build_opts []string
	debug_opts []string
	prod_opts  []string
}

fn (self Module) build(config Config) {
	mut opts := config.opts.clone()
	opts << self.build_opts
	match config.mode {
		.debug {
			opts << self.debug_opts
		}
		.default {}
		.release {
			opts << self.prod_opts
		}
	}
	cmd := '${config.cc} -o target/${self.name} ${opts.join(' ')} ${self.sources.join(' ')}'
	println('${self.name}: ${cmd}')
	system(cmd)
}

@[params]
struct FilesParams {
pub:
	path       string
	excluded   []string
	additional []string
	filter     fn (string) bool = |path| path.ends_with('.c')
}

@[inline]
fn files(params FilesParams) []string {
	paths := ls(params.path) or { panic(err) }
		.filter(it !in params.excluded && params.filter(it))
		.map('${params.path}/${it}')
	return arrays.append(paths, params.additional)
}

// Config //
config := Config{}

libhoshi := Module{
	name:       'libhoshi.so'
	sources:    files(
		path:       'src/hoshi'
		excluded:   ['main.c']
		additional: ['src/binio/binio.c']
	)
	build_opts: ['-fPIC', '-shared']
	debug_opts: [
		'-DHOSHI_ENABLE_TRACE_EXECUTION_DEBUGGING=1',
		'-DHOSHI_DISASSEMBLER_ENABLE_RAW_CODE_DUMP=1',
		'-DHOSHI_DISASSEMBLER_ENABLE_CONSTANT_DUMP=1',
		'-DHOSHI_ENABLE_CHUNK_WRITE_DEBUG_INFO=1',
		'-DHOSHI_ENABLE_CHUNK_READ_DEBUG_INFO=1',
		'-DHOSHI_ENABLE_CHUNK_DEBUG_FLAGS=1',
	]
}
hoshi := Module{
	...libhoshi
	name:       'hoshi'
	depends:    ['libhoshi.so']
	sources:    ['src/hoshi/main.c', 'target/libhoshi.so']
	build_opts: []
}
hir := Module{
	name:       'hir'
	depends:    ['libhoshi.so']
	sources:    files(
		path:       'src/hir'
		additional: ['target/libhoshi.so', 'src/common/thirdparty/asprintf.c']
	)
	debug_opts: [
		'-DHIR_ENABLE_PRINT_CODE=1',
		'-DHIR_ENABLE_PRINT_DISASSEMBLY=1',
		'-DHIR_ENABLE_TOKEN_DUMP=1',
	]
}
taiyo := Module{
	name:    'taiyo'
	sources: ls('src/taiyo')!
		.filter(it.ends_with('.c'))
		.map('src/taiyo/${it}')
}

modules := [libhoshi, hoshi, hir, taiyo]

mut context := build.context(default: 'all')

context.artifact(
	name: 'target'
	run:  |self| mkdir('target')!
)

for m in modules {
	context.task(
		name:    m.name
		depends: arrays.append(['target'], m.depends)
		run:     fn [m, config] (self build.Task) ! {
			m.build(config)
		}
	)
}

context.task(
	name:    'all'
	depends: modules.map(|it| it.name)
	run:     |self| none
)

context.run()
