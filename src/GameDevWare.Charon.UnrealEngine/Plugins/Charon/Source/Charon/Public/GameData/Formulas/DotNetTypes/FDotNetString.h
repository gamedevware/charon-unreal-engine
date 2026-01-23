#pragma once

#include "GameData/Formulas/EFormulaValueType.h"
#if __has_include("UObject/StrProperty.h")
#include "UObject/StrProperty.h"
#endif
#include "FDotNetString.generated.h"

/*
 * Internal struct used for reflection in formulas 
 */
USTRUCT()
struct FDotNetString
{
	GENERATED_BODY()

private:
	inline static FStrProperty* LiteralProperty = nullptr;
	
public:
	inline static EFormulaValueType TypeCode = EFormulaValueType::String;
	
	static FStrProperty* GetLiteralProperty()
	{
		if (LiteralProperty)
		{
			return LiteralProperty;
		}
		return LiteralProperty = CastFieldChecked<FStrProperty>(StaticStruct()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(FDotNetString, __Literal)));
	}
	
	UPROPERTY()
	FString __Literal;
};