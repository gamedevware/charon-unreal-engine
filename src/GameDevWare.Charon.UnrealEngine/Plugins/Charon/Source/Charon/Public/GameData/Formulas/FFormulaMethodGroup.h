#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "FFormulaVariableValue.h"
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
		const FFormulaVariableValue& Target,
		const TMap<FString, FFormulaVariableValue>& CallArguments,
		const FProperty* ExpectedType,
		const TArray<UClass*>& TypeArguments,
		FFormulaVariableValue& Result
	);
};
