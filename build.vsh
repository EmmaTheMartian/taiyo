#!/usr/bin/env -S v -enable-globals run

import build
import arrays
import toml

struct Config {
pub mut:
	cc         string
	mode       string
	memwatch   bool
	opts       []string
	debug_opts []string
	prod_opts  []string
}

struct Module {
pub:
	name       string
	sources    []string
	depends    []string
	build_opts []string
	debug_opts []string
	prod_opts  []string
	include    []string
}

__global config Config

fn (self Module) build() {
	// Get options
	mut opts := config.opts.clone()
	opts << self.build_opts
	match config.mode {
		'debug' {
			opts << config.debug_opts
			opts << self.debug_opts
		}
		'default' {}
		'prod' {
			opts << config.prod_opts
			opts << self.prod_opts
		}
		else {
			eprintln('error: unknown mode: ${config.mode}')
		}
	}

	if config.memwatch {
		opts << '-DMEMWATCH'
		opts << '-DMEMWATCH_STDIO'
		opts << '-Iexternal/memwatch/'
	}

	// Get sources
	mut sources := self.sources.clone()
	if config.memwatch {
		sources << 'external/memwatch/memwatch.c'
	}

	// Get includes
	includes := self.include.map('-L${it}').join(' ')

	// Run command
	cmd := '${config.cc} -o target/${self.name} ${includes} ${opts.join(' ')} ${sources.join(' ')}'
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

@[inline]
fn dir(mut context build.BuildContext, path string) {
	context.artifact(
		name: path
		run:  fn [path] (self build.Task) ! {
			mkdir(path)!
		}
	)
}

// Modules
libhoshi := Module{
	name:       'libhoshi.so'
	sources:    files(
		path:       'src/hoshi'
		excluded:   ['main.c']
		additional: ['src/hoshi/binio/binio.c']
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
	include:    [
		'thirdparty/siphash',
	]
}
hoshi := Module{
	...libhoshi
	name:    'hoshi'
	depends: ['libhoshi.so']
	build_opts: []
	sources: ['src/hoshi/main.c', 'target/libhoshi.so']
}
hir := Module{
	name:       'hir'
	depends:    ['libhoshi.so']
	sources:    files(
		path:       'src/hir'
		additional: ['target/libhoshi.so']
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

config = toml.decode[Config](read_file('build.toml')!)!
mut context := build.context(default: 'all')

dir(mut context, 'target')
dir(mut context, 'external')

for mode in ['debug', 'default', 'prod'] {
	context.task(
		name: '.${mode}',
		run:  fn [mode] (self build.Task) ! {
			config = Config{...config, mode: mode}
		}
	)
}

for m in modules {
	context.task(
		name:    m.name
		depends: arrays.append(['target'], m.depends)
		run:     fn [m] (self build.Task) ! {
			m.build()
		}
	)
}

context.task(
	name:    'all'
	depends: modules.map(|it| it.name)
	run:     |self| none
)

context.artifact(
	name:    'external/memwatch'
	depends: ['external']
	run:     fn (self build.Task) ! {
		system('git clone https://github.com/linkdata/memwatch external/memwatch/')
	}
)

context.task(
	name: 'clean'
	run:  fn (self build.Task) ! {
		rmdir_all('target') or {}
		rmdir_all('external') or {}
		rm('out.hoshi') or {}
		rm('memwatch.log') or {}
	}
)

context.run()
