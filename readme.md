# Taiyo - 太陽 (sun)

> A LISP-like, dynamically typed, metaprogrammable, and functional programming language.

**Goals:**

- Extremely powerful compile-time programming. **All** code is valid at run-time *and* compile-time.
- No garbage collector, but no manual memory management either.
- Compiles to HIR (Hoshi Intermediate Representation).
- Interoperability with C.

## Build Guide

```sh
# Build the Taiyo compiler
$ gcc -std=c11 -Wall -o taiyo src/taiyo/main.c
# Build the Hoshi runtime
$ gcc -std=c11 -Wall -o hoshi src/hoshi/main.c
```

# Hoshi - 星 (star)

> A stack-based runtime/VM designed specifically for Taiyo.

The Hoshi Runtime is how Taiyo is capable of intermingling compile-time and
run-time code so flexibly.

# Resources

- [Crafting Interpreters](https://craftinginterpreters.com/)
	- I cannot give enough praise to Robert Nystrom for this *masterpiece* of
	knowledge. If you want to make an interpreter, **absolutely** use this book. It's
	completely free on the web too <3
