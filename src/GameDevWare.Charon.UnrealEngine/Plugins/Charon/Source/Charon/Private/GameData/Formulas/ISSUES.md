## Critical — Crash / Undefined Behaviour

## High — Memory / Object Lifetime

## Medium — Correctness

## Won't fix

### `FFormulaTypeResolver.cpp:409` — Enum underlying type hardcoded to Int64

Justification: `TryCopyNumericValue` and other forms of type coercion are dynamic, so they fit by value, not by type. And Int64 is safest option to store all possible values.

### `FFormulaTypeResolver.cpp:15` — Global type cache pins UObjects forever, unsafe across Hot Reload

Justification: currently won't fix.

### `FNewExpression.cpp:71` — `new` expression creates unrooted UObject, GC-eligible immediately

Justification: any created UObject should be returned and stored by called, or being collected by GC. 
Also all values FFormulaValue live only during expression execution (one call, no async stuff) and destroyed after Invoke().