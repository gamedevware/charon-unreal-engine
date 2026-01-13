#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "Containers/Map.h"
#include "Templates/SharedPointer.h"
#include "FFormulaTypeResolver.h"
#include "FFormulaVariableValue.h"

class CHARON_API FFormulaExecutionContext
{
public:
	bool AutoNullPropagation;
	TMap<FString, FFormulaVariableValue> Arguments;
	FFormulaVariableValue Global;
	TSharedRef<FFormulaTypeResolver> TypeResolver;
	
	FFormulaExecutionContext
	(
		bool bAutoNullPropagation,
		const TMap<FString, FFormulaVariableValue>& Arguments,
		const FFormulaVariableValue& Global,
		const TSharedRef<FFormulaTypeResolver>& TypeResolver
	);
};
