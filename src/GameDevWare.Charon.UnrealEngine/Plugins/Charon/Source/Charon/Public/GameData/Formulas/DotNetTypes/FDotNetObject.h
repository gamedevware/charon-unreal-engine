#pragma once

#include "GameData/Formulas/EFormulaValueType.h"
#include "FDotNetObject.generated.h"

/*
 * Internal struct used for reflection in formulas 
 */
USTRUCT()
struct FDotNetObject
{
	GENERATED_BODY()

private:
	inline static FObjectPropertyBase* LiteralObjectProperty = nullptr;
	inline static FObjectPropertyBase* LiteralNullProperty = nullptr;

	UPROPERTY()
	UObject* __Literal = nullptr;
	UPROPERTY()
	UObject* __NullLiteral = nullptr;
public:
	inline static EFormulaValueType TypeCode = EFormulaValueType::ObjectPtr;
	
	static FObjectPropertyBase* GetLiteralProperty()
	{
		if (LiteralObjectProperty)
		{
			return LiteralObjectProperty;
		}
		return LiteralObjectProperty = CastFieldChecked<FObjectPropertyBase>(StaticStruct()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(FDotNetObject, __Literal)));
	}
	static FObjectPropertyBase* GetNullLiteralProperty()
	{
		if (LiteralNullProperty)
		{
			return LiteralNullProperty;
		}
		return LiteralNullProperty = CastFieldChecked<FObjectPropertyBase>(StaticStruct()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(FDotNetObject, __NullLiteral)));
	}
	
};