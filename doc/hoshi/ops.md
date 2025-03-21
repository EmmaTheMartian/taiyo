# Hoshi Operations

> ![NOTE]
> This file is autogenerated by [`ops.vsh`](./ops.vsh).

| Name            | C                        | HIR         | Args | Pops->Pushes | Link                   |
| --------------- | ------------------------ | ----------- | ---- | ------------ | ---------------------- |
| `PUSH`          | `HOSHI_OP_PUSH`          | `push`      | 1    | 0->1         | [more](#push)          |
| `POP`           | `HOSHI_OP_POP`           | `pop`       | 0    | 1->0         | [more](#pop)           |
| `CONSTANT`      | `HOSHI_OP_CONSTANT`      | `n/a`       | 1    | 0->1         | [more](#constant)      |
| `CONSTANT_LONG` | `HOSHI_OP_CONSTANT_LONG` | `n/a`       | 3    | 0->1         | [more](#constant_long) |
| `TRUE`          | `HOSHI_OP_TRUE`          | `true`      | 0    | 0->1         | [more](#true)          |
| `FALSE`         | `HOSHI_OP_FALSE`         | `false`     | 0    | 0->1         | [more](#false)         |
| `NIL`           | `HOSHI_OP_NIL`           | `nil`       | 0    | 0->1         | [more](#nil)           |
| `DEFGLOBAL`     | `HOSHI_OP_DEFGLOBAL`     | `defglobal` | 1    | 1->0         | [more](#defglobal)     |
| `SETGLOBAL`     | `HOSHI_OP_SETGLOBAL`     | `setglobal` | 1    | 0->0         | [more](#setglobal)     |
| `GETGLOBAL`     | `HOSHI_OP_GETGLOBAL`     | `getglobal` | 1    | 0->1         | [more](#getglobal)     |
| `ADD`           | `HOSHI_OP_ADD`           | `add`       | 0    | 2->1         | [more](#add)           |
| `SUB`           | `HOSHI_OP_SUB`           | `sub`       | 0    | 2->1         | [more](#sub)           |
| `MUL`           | `HOSHI_OP_MUL`           | `mul`       | 0    | 2->1         | [more](#mul)           |
| `DIV`           | `HOSHI_OP_DIV`           | `div`       | 0    | 2->1         | [more](#div)           |
| `NEGATE`        | `HOSHI_OP_NEGATE`        | `negate`    | 0    | 0->0         | [more](#negate)        |
| `NOT`           | `HOSHI_OP_NOT`           | `not`       | 0    | 0->0         | [more](#not)           |
| `AND`           | `HOSHI_OP_AND`           | `and`       | 0    | 2->1         | [more](#and)           |
| `OR`            | `HOSHI_OP_OR`            | `or`        | 0    | 2->1         | [more](#or)            |
| `XOR`           | `HOSHI_OP_XOR`           | `xor`       | 0    | 2->1         | [more](#xor)           |
| `EQ`            | `HOSHI_OP_EQ`            | `eq`        | 0    | 2->1         | [more](#eq)            |
| `NEQ`           | `HOSHI_OP_NEQ`           | `neq`       | 0    | 2->1         | [more](#neq)           |
| `GT`            | `HOSHI_OP_GT`            | `gt`        | 0    | 2->1         | [more](#gt)            |
| `GTEQ`          | `HOSHI_OP_GTEQ`          | `gteq`      | 0    | 2->1         | [more](#gteq)          |
| `LT`            | `HOSHI_OP_LT`            | `lt`        | 0    | 2->1         | [more](#lt)            |
| `LTEQ`          | `HOSHI_OP_LTEQ`          | `lteq`      | 0    | 2->1         | [more](#lteq)          |
| `CONCAT`        | `HOSHI_OP_CONCAT`        | `concat`    | 0    | 2->1         | [more](#concat)        |
| `PRINT`         | `HOSHI_OP_PRINT`         | `print`     | 0    | 1->1         | [more](#print)         |
| `RETURN`        | `HOSHI_OP_RETURN`        | `return`    | 0    | 0->0         | [more](#return)        |
| `EXIT`          | `HOSHI_OP_EXIT`          | `exit`      | 0    | 1->0         | [more](#exit)          |

## `PUSH`

Push the argument's value onto the stack.
Integers from 0 to 255 are the only valid values for `push`.
Mostly for internal usage.

|        |                 |
| ------ | --------------- |
| C      | `HOSHI_OP_PUSH` |
| HIR    | `push`          |
| Args   | 1               |
| Pops   | 0               |
| Pushes | 1               |

**HIR:**

```hir
push 42 # pushes `42` onto the stack
```

## `POP`

Pop one value from the stack.

|        |                |
| ------ | -------------- |
| C      | `HOSHI_OP_POP` |
| HIR    | `pop`          |
| Args   | 0              |
| Pops   | 1              |
| Pushes | 0              |

**HIR:**

```hir
42  # push a value to pop
pop # stack is now empty
```

## `CONSTANT`

Get a constant from the chunk's constant pool with the previous value as the ID.
The value is pushed onto the stack.

|        |                     |
| ------ | ------------------- |
| C      | `HOSHI_OP_CONSTANT` |
| HIR    | `n/a`               |
| Args   | 1                   |
| Pops   | 0                   |
| Pushes | 1                   |

**HIR:**

```hir
42 # `42` is inserted into the constant pool, then is pushed onto the stack.
```

## `CONSTANT_LONG`

Get a constant from the chunk's constant pool with the previous 3 values combined as the ID.
The value is pushed onto the stack.

HIR will only emit `CONSTANT_LONG` after 256 constants have been created in the current chunk.

|        |                          |
| ------ | ------------------------ |
| C      | `HOSHI_OP_CONSTANT_LONG` |
| HIR    | `n/a`                    |
| Args   | 3                        |
| Pops   | 0                        |
| Pushes | 1                        |

**HIR:**

```hir
42 # `42` is inserted into the constant pool, then is pushed onto the stack.
```

## `TRUE`

Pushes a `true` value onto the stack.

|        |                 |
| ------ | --------------- |
| C      | `HOSHI_OP_TRUE` |
| HIR    | `true`          |
| Args   | 0               |
| Pops   | 0               |
| Pushes | 1               |

**HIR:**

```hir
true # `true` is pushed onto the stack
```

## `FALSE`

Pushes a `false` value onto the stack.

|        |                  |
| ------ | ---------------- |
| C      | `HOSHI_OP_FALSE` |
| HIR    | `false`          |
| Args   | 0                |
| Pops   | 0                |
| Pushes | 1                |

**HIR:**

```hir
false # `false` is pushed onto the stack
```

## `NIL`

Pushes a `nil` value onto the stack.

|        |                |
| ------ | -------------- |
| C      | `HOSHI_OP_NIL` |
| HIR    | `nil`          |
| Args   | 0              |
| Pops   | 0              |
| Pushes | 1              |

**HIR:**

```hir
nil # `nil` is pushed onto the stack
```

## `DEFGLOBAL`

Defines a new global variable where the name is a constant ID to a string and the value is the top value of the stack.

Variables can be redefined with `DEFGLOBAL`.

|        |                      |
| ------ | -------------------- |
| C      | `HOSHI_OP_DEFGLOBAL` |
| HIR    | `defglobal`          |
| Args   | 1                    |
| Pops   | 1                    |
| Pushes | 0                    |

**HIR:**

```hir
"Hello, World!" defglobal $hello_world # create a constant named `hello_world` with `"Hello, World!"` as the value.
getglobal $hello_world print           # print `"Hello, World!"`
```

## `SETGLOBAL`

Reassigns the value of a given variable. If the variable does not exist, the VM will throw an error.

The new value is NOT popped from the stack.

|        |                      |
| ------ | -------------------- |
| C      | `HOSHI_OP_SETGLOBAL` |
| HIR    | `setglobal`          |
| Args   | 1                    |
| Pops   | 0                    |
| Pushes | 0                    |

**HIR:**

```hir
"Hello, World!" defglobal $hello_world   # create a constant named `hello_world` with `"Hello, World!"` as the value.
"Hello, Gandalf!" setglobal $hello_world # reassign the variable to `"Hello, Gandalf!"`
pop                                      # pop the new value from the stack
getglobal $hello_world print             # print `"Hello, Gandalf!"`
```

## `GETGLOBAL`

Get the value of a given global and push it onto the stack.

|        |                      |
| ------ | -------------------- |
| C      | `HOSHI_OP_GETGLOBAL` |
| HIR    | `getglobal`          |
| Args   | 1                    |
| Pops   | 0                    |
| Pushes | 1                    |

**HIR:**

```hir
"Hello, World!" defglobal $hello_world # create a constant named `hello_world` with `"Hello, World!"` as the value.
getglobal $hello_world print           # print `"Hello, Gandalf!"`
```

## `ADD`

Pop the top two numbers from the stack, add them together, then push the result onto the stack.

|        |                |
| ------ | -------------- |
| C      | `HOSHI_OP_ADD` |
| HIR    | `add`          |
| Args   | 0              |
| Pops   | 2              |
| Pushes | 1              |

**HIR:**

```hir
40 2 add # push 40 and 2 onto the stack, then call `add`, resulting in `42` on the stack.
```

## `SUB`

Pop the top two numbers from the stack, subtract the second from the first, then push the result onto the stack.

|        |                |
| ------ | -------------- |
| C      | `HOSHI_OP_SUB` |
| HIR    | `sub`          |
| Args   | 0              |
| Pops   | 2              |
| Pushes | 1              |

**HIR:**

```hir
50 8 sub # push 50 and 8 onto the stack, then call `sub`, resulting in `42` on the stack.
```

## `MUL`

Pop the top two numbers from the stack, multiply them, then push the result onto the stack.

|        |                |
| ------ | -------------- |
| C      | `HOSHI_OP_MUL` |
| HIR    | `mul`          |
| Args   | 0              |
| Pops   | 2              |
| Pushes | 1              |

**HIR:**

```hir
2 21 mul # push 2 and 21 onto the stack, then call `mul`, resulting in `42` on the stack.
```

## `DIV`

Pop the top two numbers from the stack, divide the second from the first, then push the result onto the stack.

|        |                |
| ------ | -------------- |
| C      | `HOSHI_OP_DIV` |
| HIR    | `div`          |
| Args   | 0              |
| Pops   | 2              |
| Pushes | 1              |

**HIR:**

```hir
84 2 mul # push 84 and 2 onto the stack, then call `div`, resulting in `42` on the stack.
```

## `NEGATE`

Mutate the top number of the stack to be the negative of itself.

> Note that this _mutates_ the top value, so it does not push or pop anything.

|        |                   |
| ------ | ----------------- |
| C      | `HOSHI_OP_NEGATE` |
| HIR    | `negate`          |
| Args   | 0                 |
| Pops   | 0                 |
| Pushes | 0                 |

**HIR:**

```hir
42 negate # top value is now `-42`
```

## `NOT`

Mutate the top boolean of the stack to be `true` if it was `false`, or vice-versa.

> Note that this _mutates_ the top value, so it does not push or pop anything.

|        |                |
| ------ | -------------- |
| C      | `HOSHI_OP_NOT` |
| HIR    | `not`          |
| Args   | 0              |
| Pops   | 0              |
| Pushes | 0              |

**HIR:**

```hir
true not # top value is `false`
```

## `AND`

Pop the top two booleans from the stack, if they are both `true`, then push `true` onto the stack. Otherwise push `false`.

|        |                |
| ------ | -------------- |
| C      | `HOSHI_OP_AND` |
| HIR    | `and`          |
| Args   | 0              |
| Pops   | 2              |
| Pushes | 1              |

**HIR:**

```hir
true false and # top value is `false`
```

## `OR`

Pop the top two booleans from the stack, if either are `true`, then push `true` onto the stack. Otherwise push `false`.

|        |               |
| ------ | ------------- |
| C      | `HOSHI_OP_OR` |
| HIR    | `or`          |
| Args   | 0             |
| Pops   | 2             |
| Pushes | 1             |

**HIR:**

```hir
true false or # top value is `true`
```

## `XOR`

Pop the top two booleans from the stack, if one is `true` and the other is `false`, then push `true` onto the stack. Otherwise push `false`.

|        |                |
| ------ | -------------- |
| C      | `HOSHI_OP_XOR` |
| HIR    | `xor`          |
| Args   | 0              |
| Pops   | 2              |
| Pushes | 1              |

**HIR:**

```hir
true false or # top value is `true`
```

## `EQ`

Pop the top two values from the stack, if they are equal, then push `true` onto the stack. Otherwise push `false`.

|        |               |
| ------ | ------------- |
| C      | `HOSHI_OP_EQ` |
| HIR    | `eq`          |
| Args   | 0             |
| Pops   | 2             |
| Pushes | 1             |

**HIR:**

```hir
42 42 eq # top value is `true`
```

## `NEQ`

Pop the top two values from the stack, if they are _NOT_ equal, then push `true` onto the stack. Otherwise push `false`.

|        |                |
| ------ | -------------- |
| C      | `HOSHI_OP_NEQ` |
| HIR    | `neq`          |
| Args   | 0              |
| Pops   | 2              |
| Pushes | 1              |

**HIR:**

```hir
42 12 eq # top value is `true`
```

## `GT`

Pop the top two numbers from the stack, if the first is greater than the second, then push `true` onto the stack. Otherwise push `false`.

|        |               |
| ------ | ------------- |
| C      | `HOSHI_OP_GT` |
| HIR    | `gt`          |
| Args   | 0             |
| Pops   | 2             |
| Pushes | 1             |

**HIR:**

```hir
42 12 gt # top value is `true`
```

## `GTEQ`

Pop the top two numbers from the stack, if the first is greater than or equal to the second, then push `true` onto the stack. Otherwise push `false`.

|        |                 |
| ------ | --------------- |
| C      | `HOSHI_OP_GTEQ` |
| HIR    | `gteq`          |
| Args   | 0               |
| Pops   | 2               |
| Pushes | 1               |

**HIR:**

```hir
42 12 gteq # top value is `true`
```

## `LT`

Pop the top two numbers from the stack, if the first is less than the second, then push `true` onto the stack. Otherwise push `false`.

|        |               |
| ------ | ------------- |
| C      | `HOSHI_OP_LT` |
| HIR    | `lt`          |
| Args   | 0             |
| Pops   | 2             |
| Pushes | 1             |

**HIR:**

```hir
42 12 lt # top value is `false`
```

## `LTEQ`

Pop the top two numbers from the stack, if the first is less than or equal to the second, then push `true` onto the stack. Otherwise push `false`.

|        |                 |
| ------ | --------------- |
| C      | `HOSHI_OP_LTEQ` |
| HIR    | `lteq`          |
| Args   | 0               |
| Pops   | 2               |
| Pushes | 1               |

**HIR:**

```hir
42 12 lteq # top value is `false`
```

## `CONCAT`

Pop the top two strings from the stack, then concatenate them together.

|        |                   |
| ------ | ----------------- |
| C      | `HOSHI_OP_CONCAT` |
| HIR    | `concat`          |
| Args   | 0                 |
| Pops   | 2                 |
| Pushes | 1                 |

**HIR:**

```hir
"Hello, " "World!" concat # top value is `"Hello, World!"`
```

## `PRINT`

Pop the top value from the stack and print it to stdout.

|        |                  |
| ------ | ---------------- |
| C      | `HOSHI_OP_PRINT` |
| HIR    | `print`          |
| Args   | 0                |
| Pops   | 1                |
| Pushes | 1                |

**HIR:**

```hir
"Hello, World!\n" print # `"Hello, World!\n"` is printed to stdout
```

## `RETURN`

> ![WARN]
> BEHAVIOUR IS TEMPORARY

Stop VM execution.

In the future this will set the return value of the current scope (or do something similar).

|        |                   |
| ------ | ----------------- |
| C      | `HOSHI_OP_RETURN` |
| HIR    | `return`          |
| Args   | 0                 |
| Pops   | 0                 |
| Pushes | 0                 |

**HIR:**

```hir
1 return 2 # quits execution upon reaching `return`, the top value of the stack is `1`
```

## `EXIT`

Pop the top value of the stack to use as an exit code, then stops VM execution and exits with the exit code.

|        |                 |
| ------ | --------------- |
| C      | `HOSHI_OP_EXIT` |
| HIR    | `exit`          |
| Args   | 0               |
| Pops   | 1               |
| Pushes | 0               |

**HIR:**

```hir
1 exit # exits the program with an exit code of `1`
```
