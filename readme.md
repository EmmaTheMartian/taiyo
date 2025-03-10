# Taiyo - 太陽 (sun)

> A LISP-like, dynamically typed, metaprogrammable, and functional programming language.

**Goals:**

- Extremely powerful compile-time programming. **All** code is valid at run-time _and_ compile-time.
- No garbage collector, but no manual memory management either.
- Compiles to HIR (Hoshi Intermediate Representation).
- Interoperability with C.

## Build Guide

```sh
zig build
# or without Zig:
sh build.sh libhoshi hoshi hir
```

That's it <3

# Hoshi - 星 (star)

> A stack-based runtime/VM designed specifically for Taiyo.

The Hoshi Runtime is how Taiyo is capable of intermingling compile-time and run-time code so flexibly.

## HIR - Hoshi Intermediate Representation

HIR is an IR/ASM for Hoshi. It has a simple compiler which writes the bytecode to a C file which is then compiled to a native binary.

Note that this **DOES NOT** turn Hoshi bytecode into native code, it instead stores all of the Hoshi bytecode and data in the C file, then sends those to a Hoshi VM to be executed.

### Building Examples

To build HIR examples without having to install it and Hoshi, you can use this command:

```sh
# Make sure you've built libhoshi using `sh build.sh libhoshi`
./hir -c --flags="-Isrc ./libhoshi.so" ./path/to/example.hir
```

The above command will tell HIR to add `src` as an include directory (needed for `#include <hoshi/*.h>` in the compiled C files.) and compile with `libhoshi.so`.

If you do have Hoshi installed, you can compile as normally (`hir -c ./path/to/example.hir`)

# Resources

- [Crafting Interpreters](https://craftinginterpreters.com/)
  - I cannot give enough praise to Robert Nystrom for this _masterpiece_ of knowledge.
    If you want to make an interpreter, **absolutely** use this book.
    It's completely free on the web too <3
