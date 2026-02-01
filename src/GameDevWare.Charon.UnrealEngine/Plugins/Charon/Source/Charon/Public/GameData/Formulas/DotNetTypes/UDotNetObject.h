// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "GameData/Formulas/EFormulaValueType.h"
#include "UDotNetObject.generated.h"

/*
 * Internal struct used for reflection in formulas 
 */
UCLASS(Hidden, NotPlaceable, Abstract)
class CHARON_API UDotNetObject : public UObject
{
	GENERATED_BODY()

private:
	UPROPERTY()
	UObject* __Literal = nullptr;
	UPROPERTY()
	UObject* __NullLiteral = nullptr;
	UPROPERTY()
	TArray<UObject*> __ArrayLiteral;
public:
	inline static EFormulaValueType TypeCode = EFormulaValueType::ObjectPtr;
	
	static FObjectPropertyBase* GetLiteralProperty()
	{
		static FObjectPropertyBase* LiteralObjectProperty = nullptr;
		
		if (LiteralObjectProperty)
		{
			return LiteralObjectProperty;
		}
		return LiteralObjectProperty = CastFieldChecked<FObjectPropertyBase>(StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UDotNetObject, __Literal)));
	}
	static FProperty* GetNullLiteralProperty()
	{
		static FProperty* LiteralNullProperty = nullptr;
		
		if (LiteralNullProperty)
		{
			return LiteralNullProperty;
		}
		return LiteralNullProperty = CastFieldChecked<FProperty>(StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UDotNetObject, __NullLiteral)));
	}
	static FArrayProperty* GetArrayProperty()
	{
		static FArrayProperty* ArrayProperty = nullptr;
		
		if (ArrayProperty)
		{
			return ArrayProperty;
		}
		return ArrayProperty = CastFieldChecked<FArrayProperty>(StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UDotNetObject, __ArrayLiteral)));
	}
	
};