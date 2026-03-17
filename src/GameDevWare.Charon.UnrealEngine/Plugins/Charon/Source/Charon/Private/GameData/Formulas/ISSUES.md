# Formula Interpreter — Identified Issues

Findings from a manual review of `Private/GameData/Formulas/` and related public headers,
focused on memory safety, UE object lifetime, and correctness.

---

## Critical — Crash / Undefined Behaviour

### 1. `FDotNetSurrogateType.cpp:9` — Null dereference in constructor

`FindPropertyByName` returns `nullptr` if `__Literal` is missing, which is then immediately dereferenced as a reference:

```cpp
LiteralField(*SurrogateClass->FindPropertyByName(TEXT("__Literal")))
```

Any surrogate `UClass` that lacks the `__Literal` UPROPERTY (e.g., a partially implemented type) will crash inside the member initializer list with no assertion message.

**Fix:** Check the return value and `check()` before dereferencing, or use a separate assertion step before the initializer.

---

### 2. `FFormulaProperty.cpp:123-130` — Inverted condition in struct-target getter

The `IsChildOf` guard is missing a negation (`!`). The current code **nullifies** `ContainerPtr` when the struct type **is** correct, and lets it through when the struct type **is wrong**:

```cpp
// BUG: condition is true for the VALID case, not the invalid one
if (InTarget->GetTypeCode() == EFormulaValueType::Struct &&
    InTarget->TryGetContainerAddress(ContainerPtr) &&
    GetScriptStruct(InTarget->GetType())->IsChildOf(DeclaringStruct))   // missing !
{
    ContainerPtr = nullptr; // zeroed for valid structs
}
```

Consequence: correct-type structs always return false (silent data miss), and wrong-type structs fall through to `ContainerPtrToValuePtr`, reading memory at an arbitrary offset on the wrong struct layout.

**Fix:** Add `!` before `IsChildOf`.

---

### 3. `FFormulaProperty.cpp:185-192` — Same inverted condition in struct-target setter

Identical logical inversion as issue #2, in `CreateDefaultPropertySetter`. A wrong-type struct passes the guard and `TrySetPropertyValue_InContainer` writes into the wrong memory layout.

**Fix:** Add `!` before `IsChildOf`.

---

### 4. `FDotNetSurrogateType.cpp:243` — Wrong map key when building instance property cache

Inside the extension-property loop, the entry is added under `MemberName` (the outer `TryGetProperty` lookup parameter) instead of the loop variable `PropertyName`:

```cpp
for (const auto& ExtensionProperty : ExtensionProperties)
{
    FString PropertyName = ExtensionProperty.Key;   // correct key
    ...
    this->Properties->Add(MemberName, ...);          // BUG: should be PropertyName
}
```

Every iteration overwrites the same key. After the loop `Properties` contains only the last extension property, and all others are silently lost. Only the property whose name happens to match `MemberName` (the one being looked up right now) will ever be found on subsequent calls.

**Fix:** Replace `MemberName` with `PropertyName` in the `Add` call.

---

## High — Memory / Object Lifetime

### 5. `FFormulaTypeResolver.cpp:338–411` — TOCTOU race in type cache (all `GetType` overloads)

Each `GetType` overload uses two separate lock scopes — a read scope that returns early if found, and a write scope that inserts:

```cpp
{
    FScopeLock ReadLock(&TypesByIdentityLock);
    if (auto* Cached = TypesByIdentity.Find(TypeKey)) return *Cached;
}
{
    FScopeLock WriteLock(&TypesByIdentityLock);
    return TypesByIdentity.Add(TypeKey, CreateFormulaType(...)); // no re-check
}
```

Between the two scopes, another thread can insert the same key. The write scope overwrites the entry, returning a new `TSharedRef`. Any caller that received the first `TSharedRef` now holds a different object than what is in the cache. For `FFormulaEnumType` this is especially harmful because the type stores a lambda capturing its own `this` (see issue #7).

**Fix:** Inside the write lock scope, check again with `FindOrAdd` semantics — insert only if the key is still absent.

---

### 6. `FFormulaTypeResolver.cpp:15` — Global type cache pins UObjects forever, unsafe across Hot Reload

```cpp
static TMap<UPTRINT, TSharedRef<IFormulaType>> TypesByIdentity;
```

`FFormulaUnrealType` and `FDotNetSurrogateType` hold `TStrongObjectPtr<UStruct/UClass>` which roots objects from GC. The cache is never cleared. During Hot Reload:

- The old `UClass*` is freed and a new one is allocated — potentially at the **same address**, causing the cache to return stale type metadata for the new class.
- The `TStrongObjectPtr` in the evicted entry keeps the old (reinstanced) class alive indefinitely.

**Fix:** Register a module-unload or Hot Reload delegate (`FModuleManager::Get().OnModuleChanged`) to clear `TypesByIdentity` when relevant modules are reloaded. Alternatively, validate cached entries against the current live `UClass` before returning them.

---

### 7. `FFormulaEnumType.cpp:31` — `ToString` lambda captures raw `this`

```cpp
this->Functions.Add(TEXT("ToString"), FFormulaFunction(
    [this](...) { return this->EnumToString(Target, Result); },
    ...));
```

`FFormulaFunction` stores the lambda by value inside a `TFunction`, which may be copied (e.g., when the map is rehashed or when a caller takes a copy). If the parent `FFormulaEnumType` is destroyed while a copied `FFormulaFunction` still lives (possible through the TOCTOU race in issue #5), the lambda calls through a dangling `this`.

**Fix:** Capture `TWeakPtr<FFormulaEnumType>` (from `AsWeak()`) instead of raw `this`, and return false if the pointer has expired.

---

### 8. `FFormulaValue.cpp:23–36` — Static singletons outlive their referenced `FProperty`

```cpp
TSharedRef<FFormulaValue> FFormulaValue::Null() {
    static TSharedPtr<FFormulaValue> NullValue = MakeShared<FFormulaValue>(nullptr);
    return NullValue.ToSharedRef();
}
```

`FFormulaValue` stores `FProperty& Type`, pointing at a property belonging to `UDotNetObject::StaticClass()`. The destructor calls `this->Type.DestroyValue(StructPtr)` on that property. Static-local `TSharedPtr`s are destroyed **after** `UObject` GC runs during engine/module shutdown, so the `UClass` (and its `FProperty` children) may already be freed when these singletons are finally destroyed. The result is a use-after-free in `DestroyValue`.

**Fix:** Either add these singletons to the UE root set / use a GC exclusion mechanism, or use an `FModuleManager` shutdown callback to explicitly reset them before UClass teardown.

---

## Medium — Correctness

### 9. `FDotNetSurrogateType.cpp:209` — `RightChop` removes the wrong end of the string

```cpp
FString PropertyName = FunctionName.RightChop(FunctionName.Len() - 3);
```

`RightChop(N)` removes the first `N` characters. `FunctionName.Len() - 3` is all but the last 3, so the result is the **last 3 characters** of the name (e.g., `GetHealth` → `lth`). The intent is to strip a 3-character prefix (`Get`/`Set`):

```cpp
FString PropertyName = FunctionName.RightChop(3); // correct
```

This bug means no extension property is ever found under its correct name. It also affects `GetPropertyNames()` at line 283 with the identical expression.

---

### 10. `FFormulaUnrealType.cpp:32–51` — `GetPropertyNames(bStatic)` ignores the `bStatic` parameter after first call

The implementation maintains a single `PropertyNames` cache regardless of `bStatic`:

```cpp
const TArray<FString>& FFormulaUnrealType::GetPropertyNames(bool bStatic)
{
    if (bStatic && !this->IsClass()) return NoMembers;

    if (!this->PropertyNames.IsValid())       // only one cache
    {
        // fills with ALL properties, no static/instance filter
        for (TFieldIterator<FProperty> It(Struct); It; ++It) ...
    }
    return *this->PropertyNames;
}
```

If first called with `bStatic=false`, the cache is filled with all properties. A subsequent call with `bStatic=true` returns the same list. If first called with `bStatic=true` on a `UClass`, the same happens in reverse. Either direction produces an incorrect name list for the other call.

**Fix:** Maintain separate `StaticPropertyNames` and `InstancePropertyNames` caches (matching the pattern already used for `FunctionNames` / `StaticFunctionNames`).

---

## Minor / Code Quality

### 11. `FFormulaTypeResolver.cpp:409` — Enum underlying type hardcoded to Int64

```cpp
TSharedRef<IFormulaType> UnderlyingType = GetType(UDotNetInt64::StaticClass()); // not correct, because in 99% it is int8
```

The comment acknowledges this is wrong. UE enums are overwhelmingly `uint8`. Using `Int64` as the underlying type means numeric comparisons between an enum value and a literal will always widen to 64-bit, and `TryCopyNumericValue` may reject valid assignments that fit in the actual type.

**Fix:** Inspect the `UEnum`'s `CppType` string or call `GetValueByIndex` and check its range, or walk the enum's underlying `FNumericProperty` via `UEnum::GetUnderlyingProperty()`.

---

### 12. `FFormulaTypeResolver.cpp:38–50` — Identity hash for collections uses pointer addition, not XOR/multiply

```cpp
TypeIdentity = reinterpret_cast<UPTRINT>(ArrayProp->GetClass()) + GetTypeIdentity(ArrayProp->Inner);
```

For `FMapProperty`, three values are summed. Because `FFieldClass` pointers are close together in memory, collisions between e.g. `TMap<int32, float>` and `TMap<float, int32>` are possible (key and value swap, same sum). A collision causes the wrong `IFormulaType` to be returned for a map, silently resolving member lookups against the wrong key/value types.

**Fix:** Use a commutative-safe hash combination (e.g., multiply by a large prime before adding, or use a hash-mix function).