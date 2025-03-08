# Design

## Syntax

```taiyo
# Parenthesis indicate run-time code, these expressions are compiled to Hoshi IR
(HIR).
# First argument is the function or macro
# All consecutive arguments are passed to the function/macro
(print "Hello, World!")

# Brace indicate compile-time code, these expressions are evaluated in a Hoshi
# runtime at compile time and can be used to generate run-time code.
{print "Hello, World!"}

# Expressions can be nested infinitely, and compile-time expressions can be
# included in run-time expressions, but not vice-versa.
(print (eval "Hello, World!"))
(print {eval "Hello, World!"})
{print {eval "Hello, World!"}}
{print (eval "Hello, World!")} # Syntax error
```
