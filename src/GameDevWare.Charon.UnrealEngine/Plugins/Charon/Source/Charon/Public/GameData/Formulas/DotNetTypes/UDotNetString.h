// Copyright GameDevWare, Denis Zykov 2025

// ReSharper disable CppPassValueParameterByConstReference
#pragma once

#include "GameData/Formulas/EFormulaValueType.h"
#if __has_include("UObject/StrProperty.h")
#include "UObject/StrProperty.h"
#endif
#include "UDotNetString.generated.h"

/*
 * Internal struct used for reflection in formulas 
 */
UCLASS(Hidden, NotPlaceable, Abstract)
class CHARON_API UDotNetString : public UObject
{
	GENERATED_BODY()

private:
	UPROPERTY()
	FString __Literal;
	UPROPERTY()
	TArray<FString> __ArrayLiteral;
public:
	inline static EFormulaValueType TypeCode = EFormulaValueType::String;
	
	static FStrProperty* GetLiteralProperty()
	{
		static FStrProperty* LiteralProperty = nullptr;
		
		if (LiteralProperty)
		{
			return LiteralProperty;
		}
		return LiteralProperty = CastFieldChecked<FStrProperty>(StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UDotNetString, __Literal)));
	}
	static FArrayProperty* GetArrayProperty()
	{
		static FArrayProperty* ArrayProperty = nullptr;
		
		if (ArrayProperty)
		{
			return ArrayProperty;
		}
		return ArrayProperty = CastFieldChecked<FArrayProperty>(StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UDotNetString, __ArrayLiteral)));
	}
	
	//* instance properties */

	UFUNCTION()
	static int GetLength(FString Self)
	{
		return Self.Len();
	}
	
	UFUNCTION()
	static int GetCount(FString Self)
	{
		return Self.Len();
	}
	
	/* static methods */
	
	UFUNCTION()
	static FString Join(FString Separator, TArray<FString> Values) {
		return FString::Join(Values, *Separator);
	}
};