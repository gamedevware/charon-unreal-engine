#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "Templates/SharedPointer.h"
#include "Misc/TVariant.h"
#include "Internationalization/Text.h"
#include "UObject/NameTypes.h"
#include "UObject/Object.h"
#include "Containers/UnrealString.h"

enum class EFormulaVariableType
{
	Null,
	Bool,
	UInt32,
	UInt64,
	Int32,
	Int64,
	Float,
	Double,
	FString,
	FName,
	FText,
	UObjectPointer,
};

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

public:
	EFormulaVariableType Type;

	FFormulaVariableValue();

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

	bool TryGet(bool& OutValue) const;

	FString ToString() const;

private:
	template <typename U>
	void Set(typename TIdentity<U>::Type&& Value, const EFormulaVariableType Type)
	{
		this->Value.Set<U>(Value);
		this->Type = Type;
	}

	/** Set a specifically-typed value into the variant */
	template <typename U>
	void Set(const typename TIdentity<U>::Type& Value, const EFormulaVariableType Type)
	{
		this->Value.Set<U>(Value);
		this->Type = Type;
	}
};
