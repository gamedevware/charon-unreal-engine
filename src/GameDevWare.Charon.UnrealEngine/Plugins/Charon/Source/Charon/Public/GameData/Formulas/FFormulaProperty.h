#pragma once

#include <functional>
#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "FFormulaValue.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Templates/SharedPointer.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"

class IFormulaTypeDescription;

using FFormulaPropertyGetterFunc = TFunction<bool(const TSharedRef<FFormulaValue>&, TSharedPtr<FFormulaValue>&)>;
using FFormulaPropertySetterFunc = TFunction<bool(const TSharedRef<FFormulaValue>&, const TSharedPtr<FFormulaValue>&)>;

class FFormulaProperty
{
	const TWeakObjectPtr<UField> DeclaringTypePtr;
	const bool bUseClassDefaultObject;
	const FFormulaPropertyGetterFunc GetterFunc;
	const FFormulaPropertySetterFunc SetterFunc;
public:
	FFormulaProperty(FProperty* Property, UField* DeclaringType, const bool bUseClassDefaultObject);
	FFormulaProperty(FFormulaPropertyGetterFunc GetterFunc, FFormulaPropertySetterFunc SetterFunc, UField* DeclaringType, const bool bUseClassDefaultObject);
	
	bool TryGetValue(const TSharedRef<FFormulaValue>& Target, TSharedPtr<FFormulaValue>& Result) const;
	bool TrySetValue(const TSharedRef<FFormulaValue>& Target, const TSharedPtr<FFormulaValue>& Value) const;

private:
	static FFormulaPropertyGetterFunc CreateDefaultPropertyGetter(TWeakFieldPtr<FProperty> PropertyPtr, TWeakObjectPtr<UField> DeclaringTypePtr);
	static FFormulaPropertySetterFunc CreateDefaultPropertySetter(TWeakFieldPtr<FProperty> PropertyPtr, TWeakObjectPtr<UField> DeclaringTypePtr);
};
