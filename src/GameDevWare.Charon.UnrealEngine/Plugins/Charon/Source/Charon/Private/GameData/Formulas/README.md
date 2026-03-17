# Formula Interpreter Architecture

The Formula Interpreter is a lightweight expression evaluator for Charon that allows game designers to define computed fields using formula expressions. It executes C# 3.5-style LINQ expression syntax (for historical compatibility) compiled into a graph of Unreal Engine reflection-aware expression nodes.

## Overview

The formula system is built on a **visitor pattern** where expressions are evaluated in a context that provides:
- Type resolution via reflection
- Member access (properties and methods) through Unreal's reflection system
- Type conversion and binary/unary operations
- Null propagation and error handling

### Design Philosophy

1. **Reflection-Based**: Uses Unreal Engine's reflection system (UProperty, UFunction, UClass) to resolve types and members at runtime
2. **Type-Safe**: Enforces type constraints during expression building and execution
3. **.NET Compatibility**: Simulates .NET types (that don't exist in C++) using surrogate classes
4. **C# 3.5 Syntax**: Expressions are JSON-serialized AST matching C# LINQ expression format

---

## Core Architecture

### Expression Hierarchy

```
FFormulaExpression (base)
├── FConstantExpression        // Literals (42, "hello", null, true)
├── FMemberExpression          // Property/field access (obj.Property)
├── FInvokeExpression          // Method calls (obj.Method())
├── FBinaryExpression          // Binary ops (+, -, *, /, <, >, ==, etc.)
├── FUnaryExpression           // Unary ops (!, -, ~, +)
├── FConditionExpression       // Ternary (test ? ifTrue : ifFalse)
├── FConvertExpression         // Type casting and conversion
├── FIndexExpression           // Array/collection indexing (arr[0])
├── FTypeOfExpression          // typeof(Type)
├── FTypeIsExpression          // Type checks (obj is Type)
├── FDefaultExpression         // default(Type)
├── FNewExpression             // Object instantiation
├── FNewArrayBoundExpression   // Array allocation (new Type[size])
├── FNewArrayInitExpression    // Array initialization (new Type[] { ... })
├── FLambdaExpression          // Lambda functions (x => x + 1)
├── FListInitExpression        // List initialization
├── FMemberInitExpression      // Member initialization
└── Binding Expressions
    ├── FFormulaMemberBinding           // Initialize object property
    ├── FFormulaMemberListBinding       // Initialize array/list
    ├── FFormulaElementInitBinding      // Initialize array elements
    └── FFormulaMemberAssignmentBinding // Assign to property
```

### Execution Flow

```
FFormulaExpression::Execute(Context, ExpectedType)
    └─> FFormulaExecutionResult (contains FFormulaValue + error info)
        └─> FFormulaValue (represents runtime value)
```

---

## Main Abstractions

### 1. FFormulaExecutionContext - Evaluation Scope

**Location:** `FFormulaExecutionContext.h`

The context is the "scope" in which all expressions execute. It provides:

```cpp
class FFormulaExecutionContext
{
    bool AutoNullPropagation;                    // Propagate nulls through operations
    TMap<FString, TSharedRef<FFormulaValue>> Arguments;  // Lambda/formula parameters
    TSharedRef<FFormulaValue> Global;            // Global scope (e.g., "this")
    TSharedRef<FFormulaTypeResolver> TypeResolver;      // Type resolution service
};
```

**Usage:**
- Pass to every expression's `Execute()` method
- Access parameters via `Arguments` map (for lambdas)
- Resolve types via `TypeResolver`
- Use `Global` for implicit "this" reference

**Example:**
```cpp
FFormulaExecutionContext Context(
    true,                           // auto null propagation
    Arguments,                      // formula parameters
    GlobalObject,                   // global scope
    TypeResolver                    // for resolving types
);
result = Expression->Execute(Context, ExpectedType);
```

---

### 2. FFormulaTypeResolver - Type Resolution Service

**Location:** `FFormulaTypeResolver.h`

Resolves types from various sources and caches results for performance.

```cpp
class FFormulaTypeResolver
{
    // Resolve by UField (class, struct, enum, property)
    TSharedPtr<IFormulaType> ResolveType(const FProperty* Property);
    TSharedPtr<IFormulaType> ResolveType(const UClass* Class);
    TSharedPtr<IFormulaType> ResolveType(const UScriptStruct* Struct);
    TSharedPtr<IFormulaType> ResolveType(const UEnum* Enum);

    // Resolve by string type name (e.g., "System.Int32")
    TSharedPtr<IFormulaType> ResolveTypeByName(const FString& TypeName);
};
```

**Key Responsibilities:**
- Maps Unreal reflection metadata to `IFormulaType` wrappers
- Handles collection types (Array, Set, Map) and generates wrapper types on demand
- Caches types by identity for performance (thread-safe with `FCriticalSection`)
- Routes to appropriate type implementation:
  - `FFormulaUnrealType` - for UClass, UStruct, UEnum
  - `FDotNetSurrogateType` - for simulated .NET types (float, double, etc.)
  - `FFormulaArrayType` - for arrays
  - `FFormulaMapType` - for maps
  - `FFormulaSetType` - for sets

**Example:**
```cpp
TSharedPtr<IFormulaType> IntType = TypeResolver->ResolveType(FIntProperty::StaticClass());
TSharedPtr<IFormulaType> ArrayType = TypeResolver->ResolveType(ArrayProperty);
```

---

### 3. FFormulaProperty - Property Access Abstraction

**Location:** `FFormulaProperty.h`

Wraps Unreal property access (getter/setter) for formula expressions.

```cpp
struct FFormulaProperty
{
    // Get property value from an object
    bool TryGetValue(const TSharedRef<FFormulaValue>& Target,
                     TSharedPtr<FFormulaValue>& OutValue) const;

    // Set property value on an object
    bool TrySetValue(const TSharedRef<FFormulaValue>& Target,
                     const TSharedPtr<FFormulaValue>& InValue) const;
};
```

**Key Features:**
- **Custom Getters/Setters**: Can provide lambdas instead of direct property access for computed properties
- **Class Default Objects**: Can access static properties via `bUseClassDefaultObject` flag
- **Type Safety**: Validates property type before access
- **Metadata Tracking**: Holds weak pointers to FProperty/UField, handles GC gracefully

**Internal Functions:**
- `CreateDefaultPropertyGetter()` - Standard property reflection getter
- `CreateGetterFromFunctionInvoker()` - Wrap a function call as a getter
- `CreateDefaultPropertySetter()` - Standard property reflection setter
- `CreateSetterFromFunctionInvoker()` - Wrap a function call as a setter

**Example:**
```cpp
// Direct property access
FFormulaProperty prop(FloatProperty, MyClass);
TSharedPtr<FFormulaValue> value;
prop.TryGetValue(targetObject, value);

// Custom getter
auto customGetter = FFormulaProperty::CreateGetterFromFunctionInvoker(
    [](const TSharedRef<FFormulaValue>& Target, ...) { ... }
);
FFormulaProperty customProp(FloatProperty, customGetter, customSetter, MyClass);
```

---

### 4. FFormulaFunction - Method Invocation Abstraction

**Location:** `FFormulaFunction.h`

Wraps Unreal function calls for formula expressions.

```cpp
struct FFormulaFunction
{
    // Invoke the function with arguments
    bool TryInvoke(const TSharedRef<FFormulaValue>& Target,
                   FFormulaInvokeArguments& CallArguments,
                   const UField* ExpectedType,
                   const TArray<UField*>* TypeArguments,
                   TSharedPtr<FFormulaValue>& Result) const;
};
```

**Key Features:**
- **UFunction Integration**: Reflects Unreal function metadata and parameters
- **Parameter Validation**: Validates argument count and types before invocation
- **Class Default Objects**: Can invoke static functions via class default object
- **Out Parameters**: Handles out-parameters and return values

**Example:**
```cpp
FFormulaFunction method(UObject->FindFunctionByName("MyMethod"), MyClass);
FFormulaInvokeArguments args;
args.Add(FFormulaInvokeArguments::InvokeArgument(TEXT("0"), intValue, ...));
TSharedPtr<FFormulaValue> result;
method.TryInvoke(target, args, nullptr, nullptr, result);
```

---

### 5. IFormulaType - Type Abstraction Interface

**Location:** `IFormulaType.h`

Base interface for all resolved types. Provides member access and type operations.

```cpp
class IFormulaType
{
    virtual bool CanBeNull() const = 0;
    virtual bool IsAssignableFrom(UField* Type) const = 0;
    virtual UStruct* GetTypeClassOrStruct() const = 0;
    virtual EFormulaValueType GetTypeCode() const = 0;

    // Member access
    virtual bool TryGetProperty(const FString& Name, bool bStatic,
                               const FFormulaProperty*& OutProperty) = 0;
    virtual bool TryGetFunction(const FString& Name, bool bStatic,
                               const FFormulaFunction*& OutFunction) = 0;

    // Operations
    virtual bool TryGetBinaryOperation(EBinaryOperationType Op,
                                      const FFormulaFunction*& OutOp) = 0;
    virtual bool TryGetUnaryOperation(EUnaryOperationType Op,
                                     const FFormulaFunction*& OutOp) = 0;
    virtual bool TryGetConversionOperation(const FFormulaFunction*& OutOp) = 0;
};
```

---

## .NET Type Simulation

### Why .NET Surrogate Types?

The formula syntax is C# 3.5 LINQ expression syntax (for historical/compatibility reasons). However, some .NET types (float, double, int, etc.) don't directly map to Unreal types. The solution: **FDotNetSurrogateType** - a fake UClass that provides .NET-compatible reflection metadata.

### FDotNetSurrogateType Architecture

**Location:** `FDotNetSurrogateType.h/cpp`

Wraps a surrogate UClass (e.g., `UDotNetSingle` for C# `float`) and provides:

```cpp
class FDotNetSurrogateType : public IFormulaType
{
    TStrongObjectPtr<UClass> ClassPtr;          // Points to surrogate class
    FProperty& LiteralField;                    // Field for storing literal value
    EFormulaValueType TypeCode;                 // Maps to primitive type

    TMap<FString, FFormulaFunction> Functions;  // Static and instance methods
    TMap<FString, FFormulaProperty> Properties; // Static and instance properties
};
```

**Structure:**
- Creates lazy-loaded caches of properties/functions
- Distinguishes static vs instance members
- Manages weak references to surrogate class (survives GC)

### Example: UDotNetSingle (float simulation)

**Location:** `UDotNetSingle.h`

A hidden, non-placeable UClass that simulates C# `System.Single` (float):

```cpp
UCLASS(Hidden, NotPlaceable, Abstract)
class UDotNetSingle : public UObject
{
    // Literal storage for when you create a float value
    UPROPERTY()
    float __Literal = 0;
    UPROPERTY()
    TArray<float> __ArrayLiteral;

    // Static properties (e.g., float.MaxValue)
    UPROPERTY()
    float MaxValue = std::numeric_limits<float>::max();
    UPROPERTY()
    float MinValue = std::numeric_limits<float>::min();
    UPROPERTY()
    float Pi = 3.14159274;
    UPROPERTY()
    float E = 2.71828175;

    // Static methods (e.g., System.Single.IsNaN(0.0f))
    UFUNCTION()
    static bool IsNaN(float Value);
    UFUNCTION()
    static bool IsFinite(float Value);

    // Instance methods (e.g., (0.0f).ToString())
    UFUNCTION()
    static FString ToString(float Self);
};
```

**Why this pattern?**

1. **Reflection Integration**: Unreal's reflection system only works with UClass/UPROPERTY/UFUNCTION, so we fake a class that represents the type
2. **Property Storage**: `__Literal` holds the actual float value when wrapped in FFormulaValue
3. **Static Members**: Appear as properties on the surrogate class
4. **Instance Methods**: Implemented as static functions with `Self` parameter (like Python)

### Incomplete .NET Types

Currently implemented:
- `UDotNetSingle` (float/System.Single)
- Others partially complete

Not all .NET types are fully implemented. When adding new ones:
1. Create `UDotNetXxx` class in `Public/GameData/Formulas/DotNetTypes/`
2. Add `__Literal` and `__ArrayLiteral` properties for storage
3. Expose static properties via UPROPERTY
4. Expose static methods via UFUNCTION
5. Expose instance methods via UFUNCTION with `Self` parameter (e.g., `static FString ToString(float Self)`)
6. Register in `FFormulaTypeResolver::GetSurrogateType()`

---

## Expression Serialization Format

https://gamedevware.github.io/charon/advanced/formula_expression_ast.html
---

## Execution Examples

### Example 1: Simple Property Access

```
Formula: x => x.Name

Execution:
  1. Parameter `x` is retrieved from context
  2. FMemberExpression resolves "Name" property on `x` via reflection
  3. Calls FFormulaProperty::TryGetValue() on target object
  4. Returns property value as FFormulaValue
```

### Example 2: Method Invocation with Type Conversion

```
Formula: int velocity => float.IsNaN((float)velocity)

Execution:
  1. Parameter `velocity` is retrieved from context 
  2. FConvertExpression converts int to float
  3. FInvokeExpression calls UDotNetSingle::IsNaN(float) via reflection
  4. Returns boolean result
```

### Example 3: Null Propagation

```
Formula: x => x?.IsActive

Execution:
  1. Parameter `x` is retrieved from context
  2. If `x` is null and useNullPropagation=true, short-circuit to null
  3. Otherwise, evaluate property expression
  4. Return boolean result or null
```

---

## Debugging

Use `FFormulaExpression::ToString()` to debug expression structure:
```cpp
FString ExpressionDebug = Expression->ToString();
UE_LOG(LogFormula, Warning, TEXT("Expression: %s"), *ExpressionDebug);
```

Each expression type implements `DebugPrintTo()` to produce human-readable output.