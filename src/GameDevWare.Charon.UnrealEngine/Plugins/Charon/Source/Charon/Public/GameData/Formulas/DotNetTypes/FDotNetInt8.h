#pragma once

#include "GameData/Formulas/EFormulaValueType.h"
#include "FDotNetInt8.generated.h"

/*
 * Internal struct used for reflection in formulas 
 */
USTRUCT()
struct FDotNetInt8
{
	GENERATED_BODY()

private:
	inline static FNumericProperty* LiteralProperty = nullptr;
	
public:
	inline static EFormulaValueType TypeCode = EFormulaValueType::Int8;
	
	static FNumericProperty* GetLiteralProperty()
	{
		if (LiteralProperty)
		{
			return LiteralProperty;
		}
		return LiteralProperty = CastFieldChecked<FNumericProperty>(StaticStruct()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(FDotNetInt8, __Literal)));
	}
	
	UPROPERTY()
	int8 __Literal = 0;
};