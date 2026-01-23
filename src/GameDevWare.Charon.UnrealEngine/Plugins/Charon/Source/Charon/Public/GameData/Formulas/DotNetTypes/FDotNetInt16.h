#pragma once

#include "GameData/Formulas/EFormulaValueType.h"
#include "FDotNetInt16.generated.h"

/*
 * Internal struct used for reflection in formulas 
 */
USTRUCT()
struct FDotNetInt16
{
	GENERATED_BODY()

private:
	inline static FNumericProperty* LiteralProperty = nullptr;
	
public:
	inline static EFormulaValueType TypeCode = EFormulaValueType::Int16;
	
	static FNumericProperty* GetLiteralProperty()
	{
		if (LiteralProperty)
		{
			return LiteralProperty;
		}
		return LiteralProperty = CastFieldChecked<FNumericProperty>(StaticStruct()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(FDotNetInt16, __Literal)));
	}
	
	UPROPERTY()
	int16 __Literal = 0;
};