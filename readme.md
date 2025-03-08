# Taiyo - 太陽 (sun)

> A LISP-like, dynamically typed, metaprogrammable, and functional programming language.

**Goals:**

- Extremely powerful compile-time programming. **All** code is valid at run-time _and_ compile-time.
- No garbage collector, but no manual memory management either.
- Compiles to HIR (Hoshi Intermediate Representation).
- Interoperability with C.

## Build Guide

```sh
$ zig build
```

That's it <3

# Hoshi - 星 (star)

> A stack-based runtime/VM designed specifically for Taiyo.

The Hoshi Runtime is how Taiyo is capable of intermingling compile-time and run-time code so flexibly.

# Resources

- [Crafting Interpreters](https://craftinginterpreters.com/)
  - I cannot give enough praise to Robert Nystrom for this _masterpiece_ of knowledge.
    If you want to make an interpreter, **absolutely** use this book.
    It's completely free on the web too <3
