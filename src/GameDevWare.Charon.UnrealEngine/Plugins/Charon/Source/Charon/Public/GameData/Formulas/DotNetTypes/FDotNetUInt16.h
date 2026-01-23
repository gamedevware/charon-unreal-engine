#pragma once

#include "GameData/Formulas/EFormulaValueType.h"
#include "FDotNetUInt16.generated.h"

/*
 * Internal struct used for reflection in formulas 
 */
USTRUCT()
struct FDotNetUInt16
{
	GENERATED_BODY()

private:
	inline static FNumericProperty* LiteralProperty = nullptr;
	
public:
	inline static EFormulaValueType TypeCode = EFormulaValueType::UInt16;
	
	static FNumericProperty* GetLiteralProperty()
	{
		if (LiteralProperty)
		{
			return LiteralProperty;
		}
		return LiteralProperty = CastFieldChecked<FNumericProperty>(StaticStruct()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(FDotNetUInt16, __Literal)));
	}
	
	UPROPERTY()
	uint16 __Literal = 0;
};