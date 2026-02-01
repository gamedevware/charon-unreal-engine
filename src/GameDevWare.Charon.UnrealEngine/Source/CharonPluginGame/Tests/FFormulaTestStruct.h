// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "UFormulaTestObject.h"
#include "FFormulaTestStruct.generated.h"

USTRUCT()
struct  FFormulaTestStruct
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	TArray<int32> Int32Array;

	UPROPERTY()
	TSet<int32> Int32Set;

	UPROPERTY()
	TMap<FString, int32> Int32Map;
	
	UPROPERTY()
	UFormulaTestObject* TestObjectProp;

	UPROPERTY()
	AActor* ActorProp;
	
	UPROPERTY()
	FString StringProp;

	UPROPERTY()
	FText TextProp;

	UPROPERTY()
	FName NameProp;

	UPROPERTY()
	int32 Int32Prop = 123;
};
