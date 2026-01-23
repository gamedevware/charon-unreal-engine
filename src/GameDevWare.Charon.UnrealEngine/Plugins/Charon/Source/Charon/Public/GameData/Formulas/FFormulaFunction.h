#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "FFormulaValue.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Templates/SharedPointer.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/StrongObjectPtr.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFormulaMemberGroup, Log, All);

class IFormulaTypeDescription;

using FFormulaFunctionInvokeFunc = TFunction<bool(const TSharedRef<FFormulaValue>&, const TMap<FString, TSharedRef<FFormulaValue>>&, const IFormulaTypeDescription*, const TArray<UField*>&, TSharedPtr<FFormulaValue>&)>;

class CHARON_API FFormulaFunction
{
private:
	const FFormulaFunctionInvokeFunc FunctionInvoker;
	const TWeakObjectPtr<UField> DeclaringTypePtr;
	const bool bUseClassDefaultObject;
	
public:
	FFormulaFunction(UFunction* Function, UClass* DeclaringClass, const bool bUseClassDefaultObject);
	FFormulaFunction(FFormulaFunctionInvokeFunc Function, UField* DeclaringType, const bool bUseClassDefaultObject);
	
	bool TryInvoke(
		const TSharedRef<FFormulaValue>& Target,
		const TMap<FString, TSharedRef<FFormulaValue>>& CallArguments,
		const IFormulaTypeDescription* ExpectedType,
		const TArray<UField*>& TypeArguments,
		TSharedPtr<FFormulaValue>& Result
	) const;
	
private:
	static FFormulaFunctionInvokeFunc CreateDefaultFunctionInvoker(TWeakObjectPtr<UFunction> FunctionPtr, TWeakObjectPtr<UField> DeclaringClassPtr);
};
