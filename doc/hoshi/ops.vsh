#!/usr/bin/env -S v run

// This file is responsible for generating ops.md automagically.
// To run it, you'll need V and the mdformat Python package.
//   V:        https://github.com/vlang/v
//   mdformat: https://github.com/hukkin/mdformat
// Run using `v run ./ops.vsh` in `.../doc/hoshi/`

struct Op {
	name   string
	c      string
	hir    string
	args   int
	pops   int
	pushes int
	doc    string
	hir_ex string
}

const ops = [
	// STACK OPS //
	Op{
		name:   'PUSH'
		c:      'HOSHI_OP_PUSH'
		hir:    'push'
		args:   1
		pops:   0
		pushes: 1
		doc:    "
			Push the argument's value onto the stack.
			Integers from 0 to 255 are the only valid values for `push`.
			Mostly for internal usage.
		".trim_indent()
		hir_ex: 'push 42 # pushes `42` onto the stack'
	},
	Op{
		name:   'POP'
		c:      'HOSHI_OP_POP'
		hir:    'pop'
		args:   0
		pops:   1
		pushes: 0
		doc:    'Pop one value from the stack.'
		hir_ex: '
			42  # push a value to pop
			pop # stack is now empty
		'.trim_indent()
	},
	Op{
		name:   'CONSTANT'
		c:      'HOSHI_OP_CONSTANT'
		hir:    'n/a'
		args:   1
		pops:   0
		pushes: 1
		doc:    "
			Get a constant from the chunk's constant pool with the previous value as the ID.
			The value is pushed onto the stack.
		".trim_indent()
		hir_ex: '
			42 # `42` is inserted into the constant pool, then is pushed onto the stack.
		'.trim_indent()
	},
	Op{
		name:   'CONSTANT_LONG'
		c:      'HOSHI_OP_CONSTANT_LONG'
		hir:    'n/a'
		args:   3
		pops:   0
		pushes: 1
		doc:    "
			Get a constant from the chunk's constant pool with the previous 3 values combined as the ID.
			The value is pushed onto the stack.

			HIR will only emit `CONSTANT_LONG` after 256 constants have been created in the current chunk.
		".trim_indent()
		hir_ex: '
			42 # `42` is inserted into the constant pool, then is pushed onto the stack.
		'.trim_indent()
	},
	Op{
		name:   'TRUE'
		c:      'HOSHI_OP_TRUE'
		hir:    'true'
		args:   0
		pops:   0
		pushes: 1
		doc:    'Pushes a `true` value onto the stack.'
		hir_ex: 'true # `true` is pushed onto the stack'
	},
	Op{
		name:   'FALSE'
		c:      'HOSHI_OP_FALSE'
		hir:    'false'
		args:   0
		pops:   0
		pushes: 1
		doc:    'Pushes a `false` value onto the stack.'
		hir_ex: 'false # `false` is pushed onto the stack'
	},
	Op{
		name:   'NIL'
		c:      'HOSHI_OP_NIL'
		hir:    'nil'
		args:   0
		pops:   0
		pushes: 1
		doc:    'Pushes a `nil` value onto the stack.'
		hir_ex: 'nil # `nil` is pushed onto the stack'
	},
	// VARIABLES //
	Op{
		name:   'DEFGLOBAL'
		c:      'HOSHI_OP_DEFGLOBAL'
		hir:    'defglobal'
		args:   1
		pops:   1
		pushes: 0
		doc:    '
			Defines a new global variable where the name is a constant ID to a string and the value is the top value of the stack.

			Variables can be redefined with `DEFGLOBAL`.
		'.trim_indent()
		hir_ex: '
			"Hello, World!" defglobal \$hello_world # create a constant named `hello_world` with `"Hello, World!"` as the value.
			getglobal \$hello_world print           # print `"Hello, World!"`
		'.trim_indent()
	},
	Op{
		name:   'SETGLOBAL'
		c:      'HOSHI_OP_SETGLOBAL'
		hir:    'setglobal'
		args:   1
		pops:   0
		pushes: 0
		doc:    '
			Reassigns the value of a given variable. If the variable does not exist, the VM will throw an error.

			The new value is NOT popped from the stack.
		'.trim_indent()
		hir_ex: '
			"Hello, World!" defglobal \$hello_world   # create a constant named `hello_world` with `"Hello, World!"` as the value.
			"Hello, Gandalf!" setglobal \$hello_world # reassign the variable to `"Hello, Gandalf!"`
			pop                                      # pop the new value from the stack
			getglobal \$hello_world print             # print `"Hello, Gandalf!"`
		'.trim_indent()
	},
	Op{
		name:   'GETGLOBAL'
		c:      'HOSHI_OP_GETGLOBAL'
		hir:    'getglobal'
		args:   1
		pops:   0
		pushes: 1
		doc:    'Get the value of a given global and push it onto the stack.'
		hir_ex: '
			"Hello, World!" defglobal \$hello_world # create a constant named `hello_world` with `"Hello, World!"` as the value.
			getglobal \$hello_world print           # print `"Hello, Gandalf!"`
		'.trim_indent()
	},
	// MATH //
	Op{
		name:   'ADD'
		c:      'HOSHI_OP_ADD'
		hir:    'add'
		args:   0
		pops:   2
		pushes: 1
		doc:    'Pop the top two numbers from the stack, add them together, then push the result onto the stack.'
		hir_ex: '40 2 add # push 40 and 2 onto the stack, then call `add`, resulting in `42` on the stack.'
	},
	Op{
		name:   'SUB'
		c:      'HOSHI_OP_SUB'
		hir:    'sub'
		args:   0
		pops:   2
		pushes: 1
		doc:    'Pop the top two numbers from the stack, subtract the second from the first, then push the result onto the stack.'
		hir_ex: '50 8 sub # push 50 and 8 onto the stack, then call `sub`, resulting in `42` on the stack.'
	},
	Op{
		name:   'MUL'
		c:      'HOSHI_OP_MUL'
		hir:    'mul'
		args:   0
		pops:   2
		pushes: 1
		doc:    'Pop the top two numbers from the stack, multiply them, then push the result onto the stack.'
		hir_ex: '2 21 mul # push 2 and 21 onto the stack, then call `mul`, resulting in `42` on the stack.'
	},
	Op{
		name:   'DIV'
		c:      'HOSHI_OP_DIV'
		hir:    'div'
		args:   0
		pops:   2
		pushes: 1
		doc:    'Pop the top two numbers from the stack, divide the second from the first, then push the result onto the stack.'
		hir_ex: '84 2 mul # push 84 and 2 onto the stack, then call `div`, resulting in `42` on the stack.'
	},
	Op{
		name:   'NEGATE'
		c:      'HOSHI_OP_NEGATE'
		hir:    'negate'
		args:   0
		pops:   0
		pushes: 0
		doc:    '
			Mutate the top number of the stack to be the negative of itself.

			> Note that this _mutates_ the top value, so it does not push or pop anything.
		'.trim_indent()
		hir_ex: '42 negate # top value is now `-42`'
	},
	// BOOLEAN OPS //
	Op{
		name:   'NOT'
		c:      'HOSHI_OP_NOT'
		hir:    'not'
		args:   0
		pops:   0
		pushes: 0
		doc:    '
			Mutate the top boolean of the stack to be `true` if it was `false`, or vice-versa.

			> Note that this _mutates_ the top value, so it does not push or pop anything.
		'.trim_indent()
		hir_ex: 'true not # top value is `false`'
	},
	Op{
		name:   'AND'
		c:      'HOSHI_OP_AND'
		hir:    'and'
		args:   0
		pops:   2
		pushes: 1
		doc:    'Pop the top two booleans from the stack, if they are both `true`, then push `true` onto the stack. Otherwise push `false`.'
		hir_ex: 'true false and # top value is `false`'
	},
	Op{
		name:   'OR'
		c:      'HOSHI_OP_OR'
		hir:    'or'
		args:   0
		pops:   2
		pushes: 1
		doc:    'Pop the top two booleans from the stack, if either are `true`, then push `true` onto the stack. Otherwise push `false`.'
		hir_ex: 'true false or # top value is `true`'
	},
	Op{
		name:   'XOR'
		c:      'HOSHI_OP_XOR'
		hir:    'xor'
		args:   0
		pops:   2
		pushes: 1
		doc:    'Pop the top two booleans from the stack, if one is `true` and the other is `false`, then push `true` onto the stack. Otherwise push `false`.'
		hir_ex: 'true false or # top value is `true`'
	},
	// COMPARISONS //
	Op{
		name:   'EQ'
		c:      'HOSHI_OP_EQ'
		hir:    'eq'
		args:   0
		pops:   2
		pushes: 1
		doc:    'Pop the top two values from the stack, if they are equal, then push `true` onto the stack. Otherwise push `false`.'
		hir_ex: '42 42 eq # top value is `true`'
	},
	Op{
		name:   'NEQ'
		c:      'HOSHI_OP_NEQ'
		hir:    'neq'
		args:   0
		pops:   2
		pushes: 1
		doc:    'Pop the top two values from the stack, if they are _NOT_ equal, then push `true` onto the stack. Otherwise push `false`.'
		hir_ex: '42 12 eq # top value is `true`'
	},
	Op{
		name:   'GT'
		c:      'HOSHI_OP_GT'
		hir:    'gt'
		args:   0
		pops:   2
		pushes: 1
		doc:    'Pop the top two numbers from the stack, if the first is greater than the second, then push `true` onto the stack. Otherwise push `false`.'
		hir_ex: '42 12 gt # top value is `true`'
	},
	Op{
		name:   'GTEQ'
		c:      'HOSHI_OP_GTEQ'
		hir:    'gteq'
		args:   0
		pops:   2
		pushes: 1
		doc:    'Pop the top two numbers from the stack, if the first is greater than or equal to the second, then push `true` onto the stack. Otherwise push `false`.'
		hir_ex: '42 12 gteq # top value is `true`'
	},
	Op{
		name:   'LT'
		c:      'HOSHI_OP_LT'
		hir:    'lt'
		args:   0
		pops:   2
		pushes: 1
		doc:    'Pop the top two numbers from the stack, if the first is less than the second, then push `true` onto the stack. Otherwise push `false`.'
		hir_ex: '42 12 lt # top value is `false`'
	},
	Op{
		name:   'LTEQ'
		c:      'HOSHI_OP_LTEQ'
		hir:    'lteq'
		args:   0
		pops:   2
		pushes: 1
		doc:    'Pop the top two numbers from the stack, if the first is less than or equal to the second, then push `true` onto the stack. Otherwise push `false`.'
		hir_ex: '42 12 lteq # top value is `false`'
	},
	// STRING OPS //
	Op{
		name:   'CONCAT'
		c:      'HOSHI_OP_CONCAT'
		hir:    'concat'
		args:   0
		pops:   2
		pushes: 1
		doc:    'Pop the top two strings from the stack, then concatenate them together.'
		hir_ex: '"Hello, " "World!" concat # top value is `"Hello, World!"`'
	},
	// MISC //
	Op{
		name:   'PRINT'
		c:      'HOSHI_OP_PRINT'
		hir:    'print'
		args:   0
		pops:   1
		pushes: 1
		doc:    'Pop the top value from the stack and print it to stdout.'
		hir_ex: '"Hello, World!\\n" print # `"Hello, World!\\n"` is printed to stdout'
	},
	Op{
		name:   'RETURN'
		c:      'HOSHI_OP_RETURN'
		hir:    'return'
		args:   0
		pops:   0
		pushes: 0
		doc:    '
			> ![WARN]
			> BEHAVIOUR IS TEMPORARY

			Stop VM execution.

			In the future this will set the return value of the current scope (or do something similar).
		'.trim_indent()
		hir_ex: '1 return 2 # quits execution upon reaching `return`, the top value of the stack is `1`'
	},
	Op{
		name:   'EXIT'
		c:      'HOSHI_OP_EXIT'
		hir:    'exit'
		args:   0
		pops:   1
		pushes: 0
		doc:    'Pop the top value of the stack to use as an exit code, then stops VM execution and exits with the exit code.'
		hir_ex: '1 exit # exits the program with an exit code of `1`'
	},
]

fn main() {
	mut lines := []string{}
	lines << '# Hoshi Operations\n'

	lines << '> ![NOTE]'
	lines << '> This file is autogenerated by [`ops.vsh`](./ops.vsh).\n'

	// add the table
	lines << '| Name | C | HIR | Args | Pops->Pushes | Link |'
	lines << '| ---- | - | --- | ---- | ------------ | ---- |'
	for op in ops {
		lines << '| `${op.name}` | `${op.c}` | `${op.hir}` | ${op.args} | ${op.pops}->${op.pushes} | [more](#${op.name.to_lower().replace_char(`-`, `_`, 1)}) |'
	}
	lines << ''

	// add detailed info
	for op in ops {
		lines << '## `${op.name}`\n'
		lines << '${op.doc}\n'
		lines << '|   |   |'
		lines << '| - | - |'
		lines << '| C | `${op.c}` |'
		lines << '| HIR | `${op.hir}` |'
		lines << '| Args | ${op.args} |'
		lines << '| Pops | ${op.pops} |'
		lines << '| Pushes | ${op.pushes} |'
		lines << '\n**HIR:**\n'
		lines << '```hir'
		lines << op.hir_ex
		lines << '```\n'
	}

	write_file('ops.md', lines.join('\n'))!

	// Format markdown
	system('mdformat ops.md')
}
