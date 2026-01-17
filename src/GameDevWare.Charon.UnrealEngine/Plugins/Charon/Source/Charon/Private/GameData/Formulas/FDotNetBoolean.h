#pragma once

#include "FDotNetBoolean.generated.h"

/*
 * Internal struct used for reflection in formulas 
 */
USTRUCT()
struct FDotNetBoolean
{
	GENERATED_BODY()

private:
	static FBoolProperty* LiteralProperty;
	
public:
	static FBoolProperty* GetLiteralProperty()
	{
		if (LiteralProperty)
		{
			return LiteralProperty;
		}
		return LiteralProperty = CastField<FBoolProperty>(StaticStruct()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(FDotNetBoolean, Literal)));
	}
	
	UPROPERTY()
	bool Literal = false;
};