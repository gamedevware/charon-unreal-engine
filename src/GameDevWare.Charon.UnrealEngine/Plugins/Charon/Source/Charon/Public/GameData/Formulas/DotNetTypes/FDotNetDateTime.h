#pragma once

#include "GameData/Formulas/EFormulaValueType.h"
#include "FDotNetDateTime.generated.h"

/*
 * Internal struct used for reflection in formulas 
 */
USTRUCT()
struct FDotNetDateTime
{
	GENERATED_BODY()

private:
	inline static FStructProperty* LiteralProperty = nullptr;
	
public:
	inline static EFormulaValueType TypeCode = EFormulaValueType::DateTime;
	
	static FStructProperty* GetLiteralProperty()
	{
		if (LiteralProperty)
		{
			return LiteralProperty;
		}
		return LiteralProperty = CastFieldChecked<FStructProperty>(StaticStruct()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(FDotNetDateTime, __Literal)));
	}
	
	UPROPERTY()
	FDateTime __Literal;
};