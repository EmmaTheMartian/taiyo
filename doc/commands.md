# Useful Commands in Development

## GDB

```sh
gdb -ex=r --args ./target/<executable> <args>
```

For example, to test HIR, you can run:

```sh
gdb -ex=r --args ./target/hir -r tests/hir/hello.hir
```

## Valgrind and Memwatch

```sh
./build.vsh .debug .memwatch all
valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all -s <file and options here>
```

For example, to test HIR+libhoshi for memory leaks, you can run:

```sh
valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all -s ./target/hir -r ./tests/hir/hello.hir
```
