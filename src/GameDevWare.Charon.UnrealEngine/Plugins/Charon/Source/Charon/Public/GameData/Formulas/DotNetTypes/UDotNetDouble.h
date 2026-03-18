// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "GameData/Formulas/EFormulaValueType.h"
#include "UDotNetDouble.generated.h"

/*
 * Internal struct used for reflection in formulas 
 */
UCLASS(Hidden, NotPlaceable, Abstract)
class CHARON_API UDotNetDouble : public UObject
{
	GENERATED_BODY()

private:
	
	UPROPERTY()
	double __Literal = 0;
	UPROPERTY()
	TArray<double> __ArrayLiteral;
	
public:
	inline static EFormulaValueType TypeCode = EFormulaValueType::Double;
	
	static FNumericProperty* GetLiteralProperty()
	{
		static FNumericProperty* LiteralProperty = nullptr;
		
		if (LiteralProperty)
		{
			return LiteralProperty;
		}
		return LiteralProperty = CastFieldChecked<FNumericProperty>(StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UDotNetDouble, __Literal)));
	}
	static FArrayProperty* GetArrayProperty()
	{
		static FArrayProperty* ArrayProperty = nullptr;
		
		if (ArrayProperty)
		{
			return ArrayProperty;
		}
		return ArrayProperty = CastFieldChecked<FArrayProperty>(StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UDotNetDouble, __ArrayLiteral)));
	}
	
	//* static properties */
	
	UPROPERTY()
	double MaxValue = std::numeric_limits<double>::max();
	UPROPERTY()
	double MinValue = std::numeric_limits<double>::min();
	UPROPERTY()
	double E = 2.71828175;
	UPROPERTY()
	double Epsilon = 1.401298E-45;
	UPROPERTY()
	double NegativeInfinity = -std::numeric_limits<double>::infinity();
	UPROPERTY()
	double NegativeZero = -0.0; 
	UPROPERTY()
	double Pi = 3.14159274;
	UPROPERTY()
	double PositiveInfinity = std::numeric_limits<double>::infinity();
	UPROPERTY()
	double Tau = 6.28318548;

	//* instance properties */

	/*
	 * this code will simulate `int Something;` UPROPERTY
	UFUNCTION()
	static int GetSomething(double Self);
	UFUNCTION()
	static void SetSomething(double Self, int Value);
	*/
	
	/* static methods */
	
	UFUNCTION()
	static bool IsNaN(double Value) {
		return FMath::IsNaN(Value);
	}
	UFUNCTION()
	static bool IsFinite(const double Value) {
		return FMath::IsFinite(Value);
	}
	UFUNCTION()
	static bool IsNegativeInfinity(const double Value) {
		return !FMath::IsFinite(Value) && FMath::Sign(Value) < 0;
	}
	UFUNCTION()
	static bool IsPositiveInfinity(const double Value) {
		return !FMath::IsFinite(Value) && FMath::Sign(Value) > 0;
	}

	/* instance methods */
	
	UFUNCTION()
	static FString ToString(const double Self) {
		if (IsNaN(Self)) return TEXT("NaN");
		if (IsPositiveInfinity(Self)) return TEXT("Infinity");
		if (IsNegativeInfinity(Self)) return TEXT("-Infinity");
		return FString::SanitizeFloat(Self);
	}
};