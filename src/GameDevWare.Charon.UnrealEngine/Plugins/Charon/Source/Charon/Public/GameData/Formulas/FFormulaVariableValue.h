#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "Templates/SharedPointer.h"
#include "Misc/TVariant.h"
#include "Internationalization/Text.h"
#include "UObject/NameTypes.h"
#include "UObject/Object.h"
#include "Containers/UnrealString.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFormulaVariableValue, Log, All);

using FByteArray = TArray<uint8>;

using FFormulaVariant = TVariant<
	bool,
	uint32,
	uint64,
	int32,
	int64,
	float,
	double,
	FString,
	FName,
	FText,
	UObject*,
	FByteArray, // Structs, Unknown, Types etc
	/*
		FTimespan,
		FDateTime,
		FVector,
		FVector2D,
		FRotator,
		FTransform,
		FColor,
		FLinearColor,
		FFieldPath,
	*/
	nullptr_t
>;

class CHARON_API FFormulaVariableValue
{
	
private:
	FFormulaVariant Value;
	FProperty* Type;
	
public:
	FProperty* GetFieldType() const { return this->Type; }

	FFormulaVariableValue();
	FFormulaVariableValue(FProperty* FieldType, const void* ValuePtr);
	
	static FFormulaVariableValue Create(const bool Value);
	static FFormulaVariableValue Create(const int8 Value);
	static FFormulaVariableValue Create(const int16 Value);
	static FFormulaVariableValue Create(const int32 Value);
	static FFormulaVariableValue Create(const int64 Value);
	static FFormulaVariableValue Create(const uint8 Value);
	static FFormulaVariableValue Create(const uint16 Value);
	static FFormulaVariableValue Create(const uint32 Value);
	static FFormulaVariableValue Create(const uint64 Value);
	static FFormulaVariableValue Create(const float Value);
	static FFormulaVariableValue Create(const double Value);
	static FFormulaVariableValue Create(const FString& Value);
	static FFormulaVariableValue Create(const FName Value);
	static FFormulaVariableValue Create(const FText& Value);
	static FFormulaVariableValue Create(const TObjectPtr<UObject> Value);
	static FFormulaVariableValue Create(UObject* Value);

	bool TryCopyCompleteValue(const FProperty* OutType, void* OutValue) const;
	void CopyCompleteValue(void* DestinationPtr) const;
	
	FString ToString() const;
};
