#pragma once

#include "../EFormulaValueType.h"
#include "../TFormulaTypeMap.h"

#include "FDotNetArray.generated.h"

/*
 * Internal struct used for reflection in formulas 
 */
USTRUCT()
struct FDotNetArray
{
	GENERATED_BODY()

private:
	inline static TMap<EFormulaValueType, FArrayProperty*>  LiteralArrayProperties = TMap<EFormulaValueType, FArrayProperty*>();
	
	UPROPERTY()
	TArray<bool> BoolArrayLiteral;
	UPROPERTY()
	TArray<int8> Int8ArrayLiteral;
	UPROPERTY()
	TArray<int16> Int16ArrayLiteral;
	UPROPERTY()
	TArray<int32> Int32ArrayLiteral;
	UPROPERTY()
	TArray<int64> Int64ArrayLiteral;
	UPROPERTY()
	TArray<uint8> UInt8ArrayLiteral;
	UPROPERTY()
	TArray<uint16> UInt16ArrayLiteral;
	UPROPERTY()
	TArray<uint32> UInt32ArrayLiteral;
	UPROPERTY()
	TArray<uint64> UInt64ArrayLiteral;
	UPROPERTY()
	TArray<float> FloatArrayLiteral;
	UPROPERTY()
	TArray<double> DoubleArrayLiteral;
	UPROPERTY()
	TArray<FTimespan> TimespanArrayLiteral;
	UPROPERTY()
	TArray<FDateTime> DateTimeArrayLiteral;
	UPROPERTY()
	TArray<FString> StringArrayLiteral;
	UPROPERTY()
	TArray<FText> TextArrayLiteral;
	UPROPERTY()
	TArray<FName> NameArrayLiteral;
public:
	static FArrayProperty* GetLiteralProperty(const EFormulaValueType ArrayType)
	{
		if (LiteralArrayProperties.Num() == 0)
		{
			for (TFieldIterator<FProperty> It(StaticStruct()); It; ++It)
			{
				if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(*It))
				{
					EFormulaValueType TypeCode = GetPropertyTypeCode(ArrayProperty->Inner);
					LiteralArrayProperties.Add(TypeCode, ArrayProperty);
				}
			}
		}
		return *LiteralArrayProperties.Find(ArrayType);
	}
};