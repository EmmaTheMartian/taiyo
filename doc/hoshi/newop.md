# New Operations

Operations are used in... many places of Hoshi, HIR, and Taiyo.
This means that it can be tedious to add a new one since you have to update code in so many scattered locations.

To hopefully remedy that, here's a rundown of where operations are referenced.

## Hoshi

- `chunk.h` - Operation definitions in the `hoshi_OpCode` enum.
- `debug.c` - Debug `printf`s for each operation in the `hoshi_disassembleInstruction` function.
- `vm.c` - Operation execution in the `hoshi_runNext` function.

## HIR

- `compiler.c` - Expression byte emitters in the `hir_expression` function.
- `lexer.h` - Tokens for each operation (except for a small handful, such as `CONSTANT` and `CONSTANT_LONG`) in the `hir_TokenType` enum.
- `lexer.c` - Operator trie in the `hir_operatorType` function.
- `lexer.c` - Token string representations in the `hir_printToken` function.
