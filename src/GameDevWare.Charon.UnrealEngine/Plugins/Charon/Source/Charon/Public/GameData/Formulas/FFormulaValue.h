#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "EFormulaValueType.h"
#include "Templates/SharedPointer.h"
#include "Misc/TVariant.h"
#include "Misc/Timespan.h"
#include "Misc/DateTime.h"
#include "Internationalization/Text.h"
#include "UObject/NameTypes.h"
#include "UObject/Object.h"

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
	FTimespan,
	FDateTime,
	FByteArray, // Structs, Unknown, Types etc
	/*
		FVector,
		FTransform,
		FColor,
		FLinearColor,
		FFieldPath,
	*/
	nullptr_t
>;


class CHARON_API FFormulaValue
{
private:
	FFormulaVariant Value;
	FProperty* Type;
	EFormulaValueType TypeCode;
public:
	static FFormulaValue Null() { return FFormulaValue(); }
	static EFormulaValueType GetPropertyTypeCode(const FProperty* FieldType);
	
	FProperty* GetType() const { return this->Type; }
	EFormulaValueType GetTypeCode() const { return this->TypeCode;}
	bool IsNull() const { return  this->Value.IsType<nullptr_t>(); }
	
	FFormulaValue();
	FFormulaValue(FProperty* FieldType, const void* ValuePtr);
	
	static FFormulaValue Create(const bool Value);
	static FFormulaValue Create(const int8 Value);
	static FFormulaValue Create(const int16 Value);
	static FFormulaValue Create(const int32 Value);
	static FFormulaValue Create(const int64 Value);
	static FFormulaValue Create(const uint8 Value);
	static FFormulaValue Create(const uint16 Value);
	static FFormulaValue Create(const uint32 Value);
	static FFormulaValue Create(const uint64 Value);
	static FFormulaValue Create(const float Value);
	static FFormulaValue Create(const double Value);
	static FFormulaValue Create(const FTimespan Value);
	static FFormulaValue Create(const FDateTime Value);
	static FFormulaValue Create(const FString& Value);
	static FFormulaValue Create(const FName Value);
	static FFormulaValue Create(const FText& Value);
	static FFormulaValue Create(const TObjectPtr<UObject> Value);
	static FFormulaValue Create(UObject* Value);

	/*
	 * Try to copy value with only data size and type size check (expands ints, floats if needed)
	 * doesn't (!) validate UClass references or UEnum values
	 */
	bool TryCopyCompleteValue(const FProperty* DestinationType, void* DestinationPtr) const;

	bool TryGetBool(bool& OutValue);
	FString ToString() const;

private:
	void CopyCompleteValue(void* DestinationPtr) const;
};
