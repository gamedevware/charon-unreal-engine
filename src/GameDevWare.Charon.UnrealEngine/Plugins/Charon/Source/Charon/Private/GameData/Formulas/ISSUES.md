## Critical — Crash / Undefined Behaviour


## High — Memory / Object Lifetime

## Medium — Correctness

## Won't fix

### `FFormulaTypeResolver.cpp` — Enum underlying type hardcoded to Int64

Justification: `TryCopyNumericValue` and other forms of type coercion are dynamic, so they fit by value, not by type. And Int64 is safest option to store all possible values.

### `FFormulaTypeResolver.cpp5` — Global type cache pins UObjects forever, unsafe across Hot Reload

Justification: currently won't fix.

### `FNewExpression.cpp` — `new` expression creates unrooted UObject, GC-eligible immediately

Justification: any created UObject should be stored by caller, passed to some storing function, or being collected by GC after call. 
Also all values in FFormulaValue live only during expression execution (one call, no delayed stuff) and destroyed after execution.