#pragma once

#include "GameData/Formulas/EFormulaValueType.h"
#include "FDotNetUInt8.generated.h"

/*
 * Internal struct used for reflection in formulas 
 */
USTRUCT()
struct FDotNetUInt8
{
	GENERATED_BODY()

private:
	inline static FNumericProperty* LiteralProperty = nullptr;
	
public:
	inline static EFormulaValueType TypeCode = EFormulaValueType::UInt8;
	
	static FNumericProperty* GetLiteralProperty()
	{
		if (LiteralProperty)
		{
			return LiteralProperty;
		}
		return LiteralProperty = CastFieldChecked<FNumericProperty>(StaticStruct()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(FDotNetUInt8, __Literal)));
	}
	
	UPROPERTY()
	uint8 __Literal = 0;
};