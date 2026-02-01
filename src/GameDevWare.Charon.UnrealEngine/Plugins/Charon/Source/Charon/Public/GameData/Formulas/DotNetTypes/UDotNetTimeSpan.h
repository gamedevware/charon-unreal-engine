// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "GameData/Formulas/EFormulaValueType.h"
#include "UDotNetTimeSpan.generated.h"

/*
 * Internal struct used for reflection in formulas 
 */
UCLASS(Hidden, NotPlaceable, Abstract)
class CHARON_API UDotNetTimeSpan : public UObject
{
	GENERATED_BODY()

private:
	UPROPERTY()
	FTimespan __Literal;
	UPROPERTY()
	TArray<FTimespan> __ArrayLiteral;
public:
	inline static EFormulaValueType TypeCode = EFormulaValueType::Timespan;
	
	static FStructProperty* GetLiteralProperty()
	{
		static FStructProperty* LiteralProperty = nullptr;
		
		if (LiteralProperty)
		{
			return LiteralProperty;
		}
		return LiteralProperty = CastFieldChecked<FStructProperty>(StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UDotNetTimeSpan, __Literal)));
	}
	static FArrayProperty* GetArrayProperty()
	{
		
		static FArrayProperty* ArrayProperty = nullptr;
		if (ArrayProperty)
		{
			return ArrayProperty;
		}
		return ArrayProperty = CastFieldChecked<FArrayProperty>(StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UDotNetTimeSpan, __ArrayLiteral)));
	}
};