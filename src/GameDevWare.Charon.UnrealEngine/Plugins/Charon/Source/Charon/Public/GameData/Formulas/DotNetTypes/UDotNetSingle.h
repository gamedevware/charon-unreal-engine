#pragma once

#include "GameData/Formulas/EFormulaValueType.h"
#include "limits"

#include "UDotNetSingle.generated.h"

/*
 * Internal struct used for reflection in formulas 
 */
UCLASS(Abstract)
class UDotNetSingle : public UObject
{
	GENERATED_BODY()

private:
	inline static FNumericProperty* LiteralProperty = nullptr;
	
	UPROPERTY()
	float __Literal = 0;
public:
	inline static EFormulaValueType TypeCode = EFormulaValueType::Float;
	
	static FNumericProperty* GetLiteralProperty()
	{
		if (LiteralProperty)
		{
			return LiteralProperty;
		}
		return LiteralProperty = CastFieldChecked<FNumericProperty>(StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UDotNetSingle, __Literal)));
	}

	//* static properties */
	
	UPROPERTY()
	float MaxValue = std::numeric_limits<float>::max();
	UPROPERTY()
	float MinValue = std::numeric_limits<float>::min();
	UPROPERTY()
	float E = 2.71828175;
	UPROPERTY()
	float Epsilon = 1.401298E-45;
	UPROPERTY()
	float NegativeInfinity = -std::numeric_limits<float>::infinity();
	UPROPERTY()
	float NegativeZero = -0.0; 
	UPROPERTY()
	float Pi = 3.14159274;
	UPROPERTY()
	float PositiveInfinity = std::numeric_limits<float>::infinity();
	UPROPERTY()
	float Tau = 6.28318548;

	//* instance properties */

	/*
	UFUNCTION()
	static int GetSomething(float Self);
	UFUNCTION()
	static void SetSomething(float Self, int Value);
	*/
	
	/* static methods */
	
	UFUNCTION()
	static bool IsNaN(float Value) {
		return FMath::IsNaN(Value);
	}
	UFUNCTION()
	static bool IsFinite(const float Value) {
		return FMath::IsFinite(Value);
	}
	UFUNCTION()
	static bool IsNegativeInfinity(const float Value) {
		return !FMath::IsFinite(Value) && FMath::Sign(Value) < 0;
	}
	UFUNCTION()
	static bool IsPositiveInfinity(const float Value) {
		return !FMath::IsFinite(Value) && FMath::Sign(Value) > 0;
	}

	/* instance methods */
	
	UFUNCTION()
	static FString ToString(const float Self) {
		if (IsNaN(Self)) return TEXT("NaN");
		if (IsPositiveInfinity(Self)) return TEXT("Infinity");
		if (IsNegativeInfinity(Self)) return TEXT("-Infinity");
		return FString::SanitizeFloat(Self);
	}
};
