
### Features

* **Implemented AST Execution Engine**: Added support for executing formula expressions within the Unreal Engine environment. This update introduces executors for a broad range of AST nodes.
  * **Binary Operators**: Full support for arithmetic, logical, bitwise, and comparison operations across numeric types (`float`, `int32`), `FString`, and custom types. Supported operators include:
    * Arithmetic: `+`, `-`, `*`, `/`, `%`, `**`
    * Logical & Bitwise: `&`, `|`, `^`, `&&`, `||`, `??`, `<<`, `>>`
    * Comparison: `==`, `!=`, `>`, `>=`, `<`, `<=`
  
  * **Unary Operators**: Added support for Unary Plus (`+`), Numeric Negation (`-`), Logical Not (`!`), and Integer Complement (`~`).
  
  * **Conditional Logic**: Added support for ternary expressions (`test ? ifTrue : ifFalse`).
  * **Literal & Type Handling**:
    * Support for literals (Numeric, String, Boolean, `null`).
    * Explicit type casting/conversions (e.g., `(int)`, `(UObject)`).
    * Type metadata access via `typeof()` and type validation via `is` checks.
    * Default value initialization using `default(Type)`.
  
  
  * **Collection & Member Access**:
  * Implemented index-based access for Arrays, Sets, and Maps.
    * Support for property and function invocation on `UObject` based types.
  
  * **Object Lifecycle**:
  * Added support for Array and Object instantiation (`new`).
    * **Initializers**: Added executor support for Object and List initialization syntax (e.g., `new UObject() { Property = Value }`). Note: Parser support for this syntax is currently pending.
  
  * **Lambdas**:
    * Lambda expressions are currently not supported in this iteration of the execution engine.

* Added `GetParsedXXX` helper methods for parsing Vector data types from space-separated component strings.
* Introduced a `Clear API Key` button to the Game Data Asset window's Asset menu.
* Added popup support for the Charon embedded browser, redirecting new window requests to the default OS browser.
* Removed the `WebBrowserWidget` dependency from Runtime builds to streamline production deployments.

### Fixes

* Downgraded C++ syntax to C++17 to ensure compatibility with Unreal Engine 5.3.
* Resolved a .NET SDK tool manifest path issue occurring on machines with .NET 10 SDK installed.
* Fixed a first-run error caused by `dotnet tool install` returning 1 as a success exit code.