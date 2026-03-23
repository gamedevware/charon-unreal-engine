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