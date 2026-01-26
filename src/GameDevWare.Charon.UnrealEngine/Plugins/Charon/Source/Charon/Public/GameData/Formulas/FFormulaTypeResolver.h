// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "EFormulaValueType.h"
#include "Containers/Array.h"
#include "Templates/SharedPointer.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/Class.h"

class IFormulaType;
class FFormulaTypeReference;

class CHARON_API FFormulaTypeResolver : public TSharedFromThis<FFormulaTypeResolver>
{
	TSharedPtr<FFormulaTypeResolver> Parent;
	TMap<FString, TSharedPtr<IFormulaType>> TypesByName;
	
public:
	FFormulaTypeResolver(const TSharedPtr<FFormulaTypeResolver> Parent, const TArray<UObject*>& KnownTypes);
	virtual ~FFormulaTypeResolver() = default;

	TSharedPtr<IFormulaType> GetTypeDescription(const TSharedPtr<FFormulaTypeReference>& TypeReference);
	TSharedRef<IFormulaType> GetTypeDescription(FProperty* InProperty);
	TSharedRef<IFormulaType> GetTypeDescription(UClass* InClass);
	TSharedRef<IFormulaType> GetTypeDescription(UScriptStruct* InStruct);
	TSharedRef<IFormulaType> GetTypeDescription(UEnum* InEnum);
};
