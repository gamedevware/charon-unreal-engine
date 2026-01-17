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

class CHARON_API FFormulaMethodGroup
{
private:
	TArray<TWeakObjectPtr<UFunction>> Functions;
	
public:
	FFormulaMethodGroup(); // empty group

	void AddFunction(UFunction* Function);
	
	bool TryInvoke(
		const FFormulaValue& Target,
		const TMap<FString, FFormulaValue>& CallArguments,
		const IFormulaTypeDescription* ExpectedType,
		const TArray<UClass*>& TypeArguments,
		FFormulaValue& Result
	);
};
