# Hoshi Design Notes

This document is primarily for me to design parts of Hoshi either before
implementation or so that I can rant about something that I will or will not do
in Hoshi.

## Implicit Type Conversions

The Hoshi VM will not perform any implicit type conversions whatsoever. Explicit
type conversions, yes. Implicit, no.

My reasoning for this boils down to two things: performance and
understandability.

Lets tackle understanding first:

```hir
"Hello, "
"World!"
add
```

It can be understood easily, however if I write something like:

```hir
"hello"
1
add
```

What should happen here? In many languages, an implicit type conversion is
performed, but not all. There isn't a way to tell immediately which should
happen unless you understand how the language handles implicit type conversions.

Here's another example:

```hir
1
"hello"
add
```

Should the `1` be converted to a string, or should this throw an error?

Now for my last example:

```hir
"The meaning of life is: "
20 2 mul 2 add add
```

Now things just got way more confusing. If we rewrite this with the `concat`
instruction, it looks like this:

```hir
"The meaning of life is: "
20 2 mul 2 add concat
```

Immediately it's easier to understand at a glance.

---

Now for performance:

The drawback of this implementation is that there are more instructions, however
it means that I don't have to implement checks for implicit type conversions,
which would impact performance more since:

    1. Type checks for both operands (this would have to be checked with *every*
       `add` call, which is immensely bad for performance)
    2. Create a string buffer
    3. Convert non-string type to a string
    4. Concatenate the two types together
    5. Push the new string onto the stack

---

Ultimately: With the two operations separated, the `add` operation has less
overhead, and it makes reading Hoshi bytecode much more pleasant.
