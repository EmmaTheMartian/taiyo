# Formatting

| Formatting | Rule                                                                                                                     |
| ---------- | ------------------------------------------------------------------------------------------------------------------------ |
| Indents    | Use tabs for indents and spaces for alignment                                                                            |
| Braces     | New line for function declarations and same line for everything else                                                     |
| Ordering   | `#include`s, macros, enums, structs, global variables, static functions, functions                                       |
| Naming     | Always prefix names (static or otherwise) with the project's name. I.e, files in `src/hoshi/` are prefixed with `hoshi_` |
| Pointers   | Align `*` with the _identifier_, not the type. I.e: `int *value;`, **not** `int* value;`                                 |

## Example

```c
#include <stdio.h>

typedef struct {
	int field;
} someStruct;

int main(void)
{
	if (1 == 1) {
		puts("Hello, World!");
	}
#if true
	puts("asdf");
#endif
}
```
