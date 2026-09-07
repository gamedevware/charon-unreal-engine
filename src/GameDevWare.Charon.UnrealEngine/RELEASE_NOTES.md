# Release Notes - vNext

## New Features

### Lambda Expressions in Formulas
Lambda expressions (`x => x.Level > 10`) now execute instead of failing with an "unsupported expression" error. A lambda evaluates to a closure that captures the enclosing formula arguments and global scope, so it can be passed to any method that accepts one.

### LINQ Methods on Array Properties
Added `System.Linq.Enumerable` methods for `TArray` properties in formulas, allowing expressions such as:

```csharp
Heroes.Where(h => h.Level > 10).OrderBy(h => h.Name).First().Id
Items.Sum(i => i.Price)
Tags.Any(t => t == "Boss")
```

Supported methods:

- **Scalar results** — `Any`, `All`, `Count`, `Contains`, `First`, `FirstOrDefault`, `Last`, `LastOrDefault`, `Single`, `SingleOrDefault`, `ElementAt`, `ElementAtOrDefault`, `Min`, `Max`, `Sum`, `Average`.
- **Sequence results** — `Where`, `Skip`, `Take`, `SkipWhile`, `TakeWhile`, `Distinct`, `Reverse`, `OrderBy`, `OrderByDescending`, `Concat`, `Union`, `Except`, `Intersect`, `Select`, `ToArray`, `ToList`.

Operators that preserve the element type work with arrays of structs and enums.

Current limitations:

- LINQ methods are available on `TArray` only. Convert `TSet` and `TMap` values to `TArray` before passing them into a formula.
- `ThenBy` and `ThenByDescending` are not supported. Results are materialized eagerly, so the ordering context that these methods require is not carried between calls. Use a single `OrderBy` key instead.
- `Select` cannot project to a struct or enum element type.
- `GroupBy`, `Join`, `Zip`, `ToDictionary`, and `SelectMany` are not supported.

# Release Notes - Version 2026.2.0

## New Features

### Formula Execution Support
Added the ability to execute formulas defined in game data documents directly in C++. All expression types are supported including convert, constant, condition, default, and type-is expressions.

### Vector Data Type Parsing Helpers
Added `GetParsedXXX` helper methods for Vector data types that are stored as space-separated components, making it easier to work with vector values from game data.

### Clear API Key Button
Added a "Clear API key" button to the game data asset editor window.

### Unreal Engine 5.3 Compatibility
Downgraded C++ syntax from C++20 to C++17 to support Unreal Engine 5.3 and later.

### Tutorial Videos
Added tutorial videos to help new users get started with the plugin.

## Bug Fixes

- Fixed a bug where `UFunction::Invoke()` would not correctly initialize and destroy the parameters structure.
- Fixed sub-document and collection accessors (e.g. `GetDislikeHeroes()`) to be non-caching, preventing stale data after reload.
- Added popup (new window) support for the Charon embedded browser window.
- Removed `WebBrowserWidget` from Runtime module as it is only required during development.
- Fixed error on first start due to dotnet tool install returning exit code 1 on success.
- Fixed .NET SDK tool manifest path issue.