// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "GameData/Formulas/EFormulaValueType.h"
#include "UDotNetBoolean.generated.h"

/*
 * Internal struct used for reflection in formulas 
 */
UCLASS(Hidden, NotPlaceable, Abstract)
class CHARON_API UDotNetBoolean : public UObject
{
	GENERATED_BODY()

private:
	UPROPERTY()
	bool __Literal = false;
	UPROPERTY()
	TArray<bool> __ArrayLiteral;
public:
	inline static EFormulaValueType TypeCode = EFormulaValueType::Boolean;
	
	static FBoolProperty* GetLiteralProperty()
	{
		static FBoolProperty* LiteralProperty = nullptr;
		
		if (LiteralProperty)
		{
			return LiteralProperty;
		}
		return LiteralProperty = CastFieldChecked<FBoolProperty>(StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UDotNetBoolean, __Literal)));
	}
	static FArrayProperty* GetArrayProperty()
	{
		static FArrayProperty* ArrayProperty = nullptr;
		
		if (ArrayProperty)
		{
			return ArrayProperty;
		}
		return ArrayProperty = CastFieldChecked<FArrayProperty>(StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UDotNetBoolean, __ArrayLiteral)));
	}
};