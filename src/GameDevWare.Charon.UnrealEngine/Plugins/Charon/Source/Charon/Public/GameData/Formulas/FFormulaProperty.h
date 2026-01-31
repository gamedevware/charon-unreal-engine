// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "FFormulaFunction.h"
#include "FFormulaValue.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Templates/SharedPointer.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/WeakFieldPtr.h"
#include "UObject/WeakObjectPtr.h"

class CHARON_API IFormulaType;

using FFormulaPropertyGetterFunc = TFunction<bool(const TSharedRef<FFormulaValue>&, TSharedPtr<FFormulaValue>&)>;
using FFormulaPropertySetterFunc = TFunction<bool(const TSharedRef<FFormulaValue>&, const TSharedPtr<FFormulaValue>&)>;

DECLARE_LOG_CATEGORY_EXTERN(LogFormulaProperty, Log, All);

class FFormulaProperty
{
	TWeakObjectPtr<UField> const DeclaringTypePtr;
	bool const bUseClassDefaultObject;
	FFormulaPropertyGetterFunc const GetterFunc;
	FFormulaPropertySetterFunc const SetterFunc;
	TWeakFieldPtr<FProperty> const Property;
	
public:
	FFormulaProperty(FProperty* Property, UField* DeclaringType, const bool bUseClassDefaultObject);
	FFormulaProperty(FProperty* Property, FFormulaPropertyGetterFunc GetterFunc, FFormulaPropertySetterFunc SetterFunc, UField* DeclaringType, const bool bUseClassDefaultObject);
	
	bool TryGetValue(const TSharedRef<FFormulaValue>& InTarget, TSharedPtr<FFormulaValue>& OutValue) const;
	bool TrySetValue(const TSharedRef<FFormulaValue>& InTarget, const TSharedPtr<FFormulaValue>& InValue) const;

	FProperty* GetType() const { return Property.Get(); }
	
	// Create property getter from extension function which has signature like `static Get[PropertyName]([ThisType] Self) -> [PropertyValueType]`
	static FFormulaPropertyGetterFunc CreateGetterFromFunctionInvoker(FFormulaFunctionInvokeFunc FunctionInvoker);
	// Create property setter from extension function which has signature like `static Set[PropertyName]([ThisType] Self, [PropertyValueType] Value)`
	static FFormulaPropertySetterFunc CreateSetterFromFunctionInvoker(FFormulaFunctionInvokeFunc FunctionInvoker);

	static FFormulaPropertyGetterFunc CreateDefaultPropertyGetter(TWeakFieldPtr<FProperty> PropertyPtr, TWeakObjectPtr<UField> DeclaringTypePtr);
	static FFormulaPropertySetterFunc CreateDefaultPropertySetter(TWeakFieldPtr<FProperty> PropertyPtr, TWeakObjectPtr<UField> DeclaringTypePtr);
};
