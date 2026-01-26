// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "GameData/Formulas/EFormulaValueType.h"
#include "UDotNetDateTime.generated.h"

/*
 * Internal struct used for reflection in formulas 
 */
UCLASS(Hidden, NotPlaceable, Abstract)
class CHARON_API UDotNetDateTime : public UObject
{
	GENERATED_BODY()

private:
	UPROPERTY()
	FDateTime __Literal;
	UPROPERTY()
	TArray<FDateTime> __ArrayLiteral;
	
public:
	inline static EFormulaValueType TypeCode = EFormulaValueType::DateTime;
	
	static FStructProperty* GetLiteralProperty()
	{
		static FStructProperty* LiteralProperty = nullptr;
		
		if (LiteralProperty)
		{
			return LiteralProperty;
		}
		return LiteralProperty = CastFieldChecked<FStructProperty>(StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UDotNetDateTime, __Literal)));
	}
	static FArrayProperty* GetArrayProperty()
	{
		static FArrayProperty* ArrayProperty = nullptr;
		
		if (ArrayProperty)
		{
			return ArrayProperty;
		}
		return ArrayProperty = CastFieldChecked<FArrayProperty>(StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UDotNetDateTime, __ArrayLiteral)));
	}
};