// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "EFormulaValueType.h"
#include "Containers/Array.h"
#include "Templates/SharedPointer.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/Class.h"

class FFormulaValue;
class IFormulaType;
class FFormulaTypeReference;

class CHARON_API FFormulaTypeResolver : public TSharedFromThis<FFormulaTypeResolver>
{
	TSharedPtr<FFormulaTypeResolver> Parent;
	TMap<FString, TSharedPtr<IFormulaType>> TypesByName;
	
public:
	FFormulaTypeResolver(const TSharedPtr<FFormulaTypeResolver> Parent, const TArray<UObject*>& KnownTypes);
	virtual ~FFormulaTypeResolver() = default;

	TSharedPtr<IFormulaType> FindType(const TSharedPtr<FFormulaTypeReference>& TypeReference);
	
	TSharedRef<IFormulaType> GetType(const TSharedRef<FFormulaValue>& InValue);
	TSharedRef<IFormulaType> GetType(FProperty* InProperty);
	TSharedRef<IFormulaType> GetType(UClass* InClass);
	TSharedRef<IFormulaType> GetType(UScriptStruct* InStruct);
	TSharedRef<IFormulaType> GetType(UEnum* InEnum);
};
