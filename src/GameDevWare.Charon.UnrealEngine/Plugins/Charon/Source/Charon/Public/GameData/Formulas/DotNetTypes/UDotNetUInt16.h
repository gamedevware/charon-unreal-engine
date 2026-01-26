// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "GameData/Formulas/EFormulaValueType.h"
#include "UDotNetUInt16.generated.h"

/*
 * Internal struct used for reflection in formulas 
 */
UCLASS(Hidden, NotPlaceable, Abstract)
class CHARON_API UDotNetUInt16 : public UObject
{
	GENERATED_BODY()

private:
	UPROPERTY()
	uint16 __Literal = 0;
	UPROPERTY()
	TArray<uint16> __ArrayLiteral;
public:
	inline static EFormulaValueType TypeCode = EFormulaValueType::UInt16;
	
	static FNumericProperty* GetLiteralProperty()
	{
		static FNumericProperty* LiteralProperty = nullptr;
		
		if (LiteralProperty)
		{
			return LiteralProperty;
		}
		return LiteralProperty = CastFieldChecked<FNumericProperty>(StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UDotNetUInt16, __Literal)));
	}
	static FArrayProperty* GetArrayProperty()
	{
		static FArrayProperty* ArrayProperty = nullptr;
		
		if (ArrayProperty)
		{
			return ArrayProperty;
		}
		return ArrayProperty = CastFieldChecked<FArrayProperty>(StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UDotNetUInt16, __ArrayLiteral)));
	}
};