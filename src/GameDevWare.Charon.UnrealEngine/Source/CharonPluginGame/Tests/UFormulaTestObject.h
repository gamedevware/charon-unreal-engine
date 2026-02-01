// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "Sound/AmbientSound.h"
#include "UFormulaTestObject.generated.h"

UCLASS()
class UFormulaTestObject : public UObject
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
	
	UFUNCTION()
	inline int32 TestFunctionUint8Param(uint8 InValue) const { return this->Int32Prop + InValue; }

	UFUNCTION()
	inline int32 TestFunctionNoParam() const { return this->Int32Prop; }

	UFUNCTION()
	inline int32 TestFunctionOutParam(int32& OutParameter) const { OutParameter = this->Int32Prop; return OutParameter; }
	
	UFUNCTION()
	static inline int32 StaticTestFunctionNoParam() { return StaticClass()->GetDefaultObject<UFormulaTestObject>()->Int32Prop; }
};
