// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "GameData/Formulas/EFormulaValueType.h"
#include "UDotNetUInt32.generated.h"

/*
 * Internal struct used for reflection in formulas 
 */
UCLASS(Hidden, NotPlaceable, Abstract)
class CHARON_API UDotNetUInt32 : public UObject
{
	GENERATED_BODY()

private:
	UPROPERTY()
	uint32 __Literal = 0;
	UPROPERTY()
	TArray<uint32> __ArrayLiteral;
	
public:
	inline static EFormulaValueType TypeCode = EFormulaValueType::UInt32;
	
	static FNumericProperty* GetLiteralProperty()
	{
		static FNumericProperty* LiteralProperty = nullptr;
		
		if (LiteralProperty)
		{
			return LiteralProperty;
		}
		return LiteralProperty = CastFieldChecked<FNumericProperty>(StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UDotNetUInt32, __Literal)));
	}
	static FArrayProperty* GetArrayProperty()
	{
		static FArrayProperty* ArrayProperty = nullptr;
		
		if (ArrayProperty)
		{
			return ArrayProperty;
		}
		return ArrayProperty = CastFieldChecked<FArrayProperty>(StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UDotNetUInt32, __ArrayLiteral)));
	}
};