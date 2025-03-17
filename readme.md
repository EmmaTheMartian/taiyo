# Taiyo

> This repository contains the source code for Taiyo, Hoshi, and HIR.

**Table of Contents:**

1. [Taiyo - 太陽 (Sun)](#taiyo---太陽-sun)

   a. [Build Guide](#build-guide)

2. [Hoshi - 星 (Star)](#hoshi---星-star)
3. [HIR (Hoshi Intermediate Representation)](#hir---hoshi-intermediate-representation)
4. [File Structure](#file-structure)
5. [Resources](#resources)
6. [Credits and Attributions](#credits-and-attributions)

## Taiyo - 太陽 (sun)

> A LISP-like, dynamically typed, metaprogrammable, and functional programming language.

**Goals:**

- Extremely powerful compile-time programming. **All** code is valid at run-time _and_ compile-time.
- No garbage collector, but no manual memory management either.
- Compiles to HIR (Hoshi Intermediate Representation).
- Interoperability with C.

### Build Guide

```sh
./build.vsh all
# or without V:
sh build.sh libhoshi hoshi hir
```

That's it <3

## Hoshi - 星 (star)

> A stack-based runtime/VM designed specifically for Taiyo.

The Hoshi VM is what makes Taiyo capable of weaving compile-time and run-time code together.

## HIR - Hoshi Intermediate Representation

HIR is an IR/ASM for Hoshi. It has a simple compiler which writes the bytecode to a .hoshi file, which is then given to Hoshi for execution.

## File Structure

```
doc/ - documentation for Taiyo, Hoshi, and HIR
external/ - external, non-submodule repos; this folder is in .gitignore
src/
  hir/ - source code for HIR
  hoshi/ - source code for Hoshi
  taiyo/ - source code for Taiyo
tests/
  hir/ - tests for HIR
  hoshi/ - tests for Hoshi
  taiyo/ - tests for Taiyo
```

## Resources

- [Crafting Interpreters](https://craftinginterpreters.com/)
  - I cannot give enough praise to Robert Nystrom for this _masterpiece_ of knowledge.
    If you want to make an interpreter, **absolutely** use this book.
    It's completely free on the web too <3

## Credits and Attributions

- SipHash C Implementation <https://github.com/majek/csiphash/>, license: MIT
