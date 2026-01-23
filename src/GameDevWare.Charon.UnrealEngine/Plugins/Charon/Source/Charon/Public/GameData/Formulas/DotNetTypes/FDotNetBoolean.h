#pragma once

#include "GameData/Formulas/EFormulaValueType.h"
#include "FDotNetBoolean.generated.h"

/*
 * Internal struct used for reflection in formulas 
 */
USTRUCT()
struct FDotNetBoolean
{
	GENERATED_BODY()

private:
	inline static FBoolProperty* LiteralProperty = nullptr;
	
public:
	inline static EFormulaValueType TypeCode = EFormulaValueType::Boolean;
	
	static FBoolProperty* GetLiteralProperty()
	{
		if (LiteralProperty)
		{
			return LiteralProperty;
		}
		return LiteralProperty = CastFieldChecked<FBoolProperty>(StaticStruct()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(FDotNetBoolean, __Literal)));
	}
	
	UPROPERTY()
	bool __Literal = false;
};