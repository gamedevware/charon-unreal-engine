#pragma once

#include "GameData/Formulas/EFormulaValueType.h"
#include "FDotNetText.generated.h"

/*
 * Internal struct used for reflection in formulas 
 */
USTRUCT()
struct FDotNetText
{
	GENERATED_BODY()

private:
	inline static FTextProperty* LiteralProperty = nullptr;
	
public:
	inline static EFormulaValueType TypeCode = EFormulaValueType::Text;
	
	static FTextProperty* GetLiteralProperty()
	{
		if (LiteralProperty)
		{
			return LiteralProperty;
		}
		return LiteralProperty = CastFieldChecked<FTextProperty>(StaticStruct()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(FDotNetText, __Literal)));
	}
	
	UPROPERTY()
	FText __Literal;
};