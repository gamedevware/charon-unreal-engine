// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "GameData/Formulas/EFormulaValueType.h"
#include "UDotNetUInt64.generated.h"

/*
 * Internal struct used for reflection in formulas 
 */
UCLASS(Hidden, NotPlaceable, Abstract)
class CHARON_API UDotNetUInt64 : public UObject
{
	GENERATED_BODY()

private:
	inline static FNumericProperty* LiteralProperty = nullptr;
	inline static FArrayProperty* ArrayProperty = nullptr;

	UPROPERTY()
	uint64 __Literal = 0;
	UPROPERTY()
	TArray<uint64> __ArrayLiteral;
	
public:
	inline static EFormulaValueType TypeCode = EFormulaValueType::UInt64;
	
	static FNumericProperty* GetLiteralProperty()
	{
		if (LiteralProperty)
		{
			return LiteralProperty;
		}
		return LiteralProperty = CastFieldChecked<FNumericProperty>(StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UDotNetUInt64, __Literal)));
	}
	static FArrayProperty* GetArrayProperty()
	{
		if (ArrayProperty)
		{
			return ArrayProperty;
		}
		return ArrayProperty = CastFieldChecked<FArrayProperty>(StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UDotNetUInt64, __ArrayLiteral)));
	}
};