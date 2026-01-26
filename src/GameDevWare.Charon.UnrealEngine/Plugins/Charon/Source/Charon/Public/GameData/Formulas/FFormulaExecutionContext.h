// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "Containers/Map.h"
#include "Templates/SharedPointer.h"
#include "FFormulaTypeResolver.h"
#include "FFormulaValue.h"

class CHARON_API FFormulaExecutionContext
{
public:
	const bool AutoNullPropagation;
	TMap<FString, const TSharedRef<FFormulaValue>> Arguments; // TODO update OUT parameters when they are used in FFormulaFunction::Invoke
	const TSharedRef<FFormulaValue> Global;
	const TSharedRef<FFormulaTypeResolver> TypeResolver;
	
	FFormulaExecutionContext
	(
		bool bAutoNullPropagation,
		const TMap<FString, const TSharedRef<FFormulaValue>>& Arguments,
		const TSharedRef<FFormulaValue>& Global,
		const TSharedRef<FFormulaTypeResolver>& TypeResolver
	);
};
