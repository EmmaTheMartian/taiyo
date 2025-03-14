# Taiyo Design Notes

This document is primarily for me to design parts of Taiyo either before
implementation or so that I can rant about something that I will or will not do
in Taiyo.

## Syntax

```taiyo
# Parenthesis indicate run-time code, these expressions are compiled to Hoshi IR
(HIR).
# First argument is the function or macro
# All consecutive arguments are passed to the function/macro
(print "Hello, World!")

# Brace indicate compile-time code, these expressions are evaluated in a Hoshi
# run-time at compile time and can be used to generate run-time code.
{print "Hello, World!"}

# Expressions can be nested infinitely, and compile-time expressions can be
# included in run-time expressions, but not vice-versa.
(print (eval "Hello, World!"))
(print {eval "Hello, World!"})
{print {eval "Hello, World!"}}
{print (eval "Hello, World!")} # Syntax error
```
