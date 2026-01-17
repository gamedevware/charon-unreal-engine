#pragma once

#include "FFormulaVariableValueStruct.generated.h"

/*
 * Internal struct used for reflection in FFormulaVariableValue 
 */
USTRUCT()
struct FFormulaVariableValueStruct
{
	GENERATED_BODY()
	
	UPROPERTY()
	bool BoolValue = false;
	UPROPERTY()
	uint32 UInt32Value = 0;
	UPROPERTY()
	uint64 UInt64Value = 0;
	UPROPERTY()
	int32 Int32Value = 0;
	UPROPERTY()
	int64 Int64Value = 0;
	UPROPERTY()
	float FloatValue = 0;
	UPROPERTY()
	double DoubleValue = 0;
	UPROPERTY()
	FTimespan FTimespanValue;
	UPROPERTY()
	FDateTime FDateTimeValue;
	UPROPERTY()
	FString FStringValue;
	UPROPERTY()
	FName FNameValue;
	UPROPERTY()
	FText FTextValue;
	UPROPERTY()
	UObject* ObjectPtrValue = nullptr;
	UPROPERTY()
	UObject* NullPtrValue = nullptr;
};
