#pragma once

#include "GameData/Formulas/EFormulaValueType.h"
#include "FDotNetName.generated.h"

/*
 * Internal struct used for reflection in formulas 
 */
USTRUCT()
struct FDotNetName
{
	GENERATED_BODY()

private:
	inline static FNameProperty* LiteralProperty = nullptr;
	
public:
	inline static EFormulaValueType TypeCode = EFormulaValueType::Name;
	
	static FNameProperty* GetLiteralProperty()
	{
		if (LiteralProperty)
		{
			return LiteralProperty;
		}
		return LiteralProperty = CastFieldChecked<FNameProperty>(StaticStruct()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(FDotNetName, __Literal)));
	}
	
	UPROPERTY()
	FName __Literal;
};