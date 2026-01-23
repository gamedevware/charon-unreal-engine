#pragma once

#include "GameData/Formulas/EFormulaValueType.h"
#include "FDotNetTimeSpan.generated.h"

/*
 * Internal struct used for reflection in formulas 
 */
USTRUCT()
struct FDotNetTimeSpan
{
	GENERATED_BODY()

private:
	inline static FStructProperty* LiteralProperty = nullptr;
	
public:
	inline static EFormulaValueType TypeCode = EFormulaValueType::Timespan;
	
	static FStructProperty* GetLiteralProperty()
	{
		if (LiteralProperty)
		{
			return LiteralProperty;
		}
		return LiteralProperty = CastFieldChecked<FStructProperty>(StaticStruct()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(FDotNetTimeSpan, __Literal)));
	}
	
	UPROPERTY()
	FTimespan __Literal;
};