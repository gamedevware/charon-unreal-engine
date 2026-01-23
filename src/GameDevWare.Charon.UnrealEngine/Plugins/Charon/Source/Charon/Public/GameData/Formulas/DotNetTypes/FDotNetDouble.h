#pragma once

#include "GameData/Formulas/EFormulaValueType.h"
#include "FDotNetDouble.generated.h"

/*
 * Internal struct used for reflection in formulas 
 */
USTRUCT()
struct FDotNetDouble
{
	GENERATED_BODY()

private:
	inline static FNumericProperty* LiteralProperty = nullptr;
	
public:
	inline static EFormulaValueType TypeCode = EFormulaValueType::Float;
	
	static FNumericProperty* GetLiteralProperty()
	{
		if (LiteralProperty)
		{
			return LiteralProperty;
		}
		return LiteralProperty = CastFieldChecked<FNumericProperty>(StaticStruct()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(FDotNetDouble, __Literal)));
	}
	
	UPROPERTY()
	double __Literal = 0;
};