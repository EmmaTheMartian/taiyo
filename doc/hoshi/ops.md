# Hoshi Operations

| Name            | C                        | HIR      | Args | Pop->Push | Link                   |
| --------------- | ------------------------ | -------- | ---- | --------- | ---------------------- |
| `PUSH`          | `HOSHI_OP_PUSH`          | `push`   | 1    | 0->1      | [more](#push)          |
| `POP`           | `HOSHI_OP_POP`           | `pop`    | 0    | 1->0      | [more](#pop)           |
| `CONSTANT`      | `HOSHI_OP_CONSTANT`      | n/a      | 1    | 0->1      | [more](#constant)      |
| `CONSTANT_LONG` | `HOSHI_OP_CONSTANT_LONG` | n/a      | 3    | 0->1      | [more](#constant_long) |
| `ADD`           | `HOSHI_OP_ADD`           | `add`    | 0    | 2->1      | [more](#add)           |
| `SUB`           | `HOSHI_OP_SUB`           | `sub`    | 0    | 2->1      | [more](#sub)           |
| `MUL`           | `HOSHI_OP_MUL`           | `mul`    | 0    | 2->1      | [more](#mul)           |
| `DIV`           | `HOSHI_OP_DIV`           | `div`    | 0    | 2->1      | [more](#div)           |
| `NEGATE`        | `HOSHI_OP_NEGATE`        | `negate` | 0    | 1->1      | [more](#negate)        |
| `RETURN`        | `HOSHI_OP_RETURN`        | `return` | 0    | 1->0      | [more](#return)        |
| `EXIT`          | `HOSHI_OP_RETURN`        | `exit`   | 0    | 1->0      | [more](#exit)          |

## `PUSH`

Push one value onto the stack.

|           |                 |
| --------- | --------------- |
| C         | `HOSHI_OP_PUSH` |
| HIR       | `push`          |
| Args      | 0               |
| Push->Pop | 0->1            |

**HIR:**

```hir
push 42 // push a value
```

## `POP`

Pop one value from the stack.

|           |                |
| --------- | -------------- |
| C         | `HOSHI_OP_POP` |
| HIR       | `pop`          |
| Args      | 0              |
| Push->Pop | 1->0           |

**HIR:**

```hir
42 // push a value to pop
pop // stack is empty now
```

## `CONSTANT`

Get a constant from the chunk's constant pool with the previous value as the ID. The value is pushed onto the stack.

|           |                     |
| --------- | ------------------- |
| C         | `HOSHI_OP_CONSTANT` |
| HIR       | N/A                 |
| Args      | 1                   |
| Push->Pop | 0->1                |

**HIR:**

```hir
42 // `42` is inserted into the constant pool, then is pushed onto the stack.
```

## `CONSTANT_LONG`

Get a constant from the chunk's constant pool with the previous 3 values combined as the ID. The value is pushed onto the stack.

This is only emitted after 256 constants have been created in the current chunk.

|           |                          |
| --------- | ------------------------ |
| C         | `HOSHI_OP_CONSTANT_LONG` |
| HIR       | N/A                      |
| Args      | 3                        |
| Push->Pop | 0->1                     |

**HIR:**

```hir
42 // `42` is inserted into the constant pool, then is pushed onto the stack.
```

## `ADD`

Pop the top two values from the stack, add them together, then pushes the result onto the stack.

|           |                |
| --------- | -------------- |
| C         | `HOSHI_OP_ADD` |
| HIR       | `add`          |
| Args      | 0              |
| Push->Pop | 2->1           |

**HIR:**

```hir
40 2 add // Push 40 and 2 onto the stack, then call `add`, resulting in `42` on the stack.
```

## `SUB`

Pop the top two values from the stack, subtract the second from the first, then pushes the result onto the stack.

|           |                |
| --------- | -------------- |
| C         | `HOSHI_OP_DIV` |
| HIR       | `div`          |
| Args      | 0              |
| Push->Pop | 2->1           |

**HIR:**

```hir
50 8 sub // Push 50 and 8 onto the stack, then call `sub`, resulting in `42` on the stack.
```

## `MUL`

Pop the top two values from the stack, multiplies them, then pushes the result onto the stack.

|           |                |
| --------- | -------------- |
| C         | `HOSHI_OP_MUL` |
| HIR       | `mul`          |
| Args      | 0              |
| Push->Pop | 2->1           |

**HIR:**

```hir
2 21 mul // Push 2 and 21 onto the stack, then call `mul`, resulting in `42` on the stack.
```

## `DIV`

Pop the top two values from the stack, divides the second from the first, then pushes the result onto the stack.

|           |                |
| --------- | -------------- |
| C         | `HOSHI_OP_DIV` |
| HIR       | `div`          |
| Args      | 0              |
| Push->Pop | 2->1           |

**HIR:**

```hir
84 2 mul // Push 84 and 2 onto the stack, then call `div`, resulting in `42` on the stack.
```

## `NEGATE`

Mutate the top value of the stack to be the negative of itself.

> Note that this _mutates_ the top value, it does not push/pop.
> This should not matter, but pointing it out is still not a bad idea :P

|           |                   |
| --------- | ----------------- |
| C         | `HOSHI_OP_NEGATE` |
| HIR       | `negate`          |
| Args      | 0                 |
| Push->Pop | 0->0              |

**HIR:**

```hir
42 negate // top value is now `-42`
```

## `RETURN`

> ![WARN]
> BEHAVIOUR IS TEMPORARY

Pops and prints the value of the top element on the stack. This will be changed in the near future to return from a function.

|           |                   |
| --------- | ----------------- |
| C         | `HOSHI_OP_RETURN` |
| HIR       | `return`          |
| Args      | 0                 |
| Push->Pop | 1->0              |

**HIR:**

```hir
42 return // prints `42` to stdout
```

## `EXIT`

Pops the top value of the stack and `exit`s using that value as the return code. This **MUST** be called at the end of every program to prevent it from looping and going into a state of undefined behaviour.

|           |                 |
| --------- | --------------- |
| C         | `HOSHI_OP_EXIT` |
| HIR       | `exit`          |
| Args      | 0               |
| Push->Pop | 1->0            |

**HIR:**

```hir
42 exit // exits with `42` as the exit code
```
