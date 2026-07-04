// Copyright GameDevWare, Denis Zykov 2025

// ReSharper disable CppPassValueParameterByConstReference
#pragma once
#include <cmath>
#undef PI

#include "UDotNetMath.generated.h"

/*
 * Internal struct used for reflection in formulas
 */
UCLASS(Hidden, NotPlaceable, Abstract)
class CHARON_API UDotNetMath : public UObject
{
	GENERATED_BODY()

public:

	/* static properties */

	UPROPERTY()
	double E = 2.718281828459045;

	UPROPERTY()
	double PI = 3.141592653589793;

	/* static methods */

	UFUNCTION()
	static double Truncate(double D) {
		return std::trunc(D);
	}

	UFUNCTION()
	static double IEEERemainder(double X, double Y) {
		return std::remainder(X, Y);
	}

	UFUNCTION()
	static double Floor(double D) {
		return std::floor(D);
	}

	UFUNCTION()
	static double Ceiling(double D) {
		return std::ceil(D);
	}

	UFUNCTION()
	static double Acos(double D) {
		return std::acos(D);
	}

	UFUNCTION()
	static double Acosh(double D) {
		return std::acosh(D);
	}

	UFUNCTION()
	static double Asin(double D) {
		return std::asin(D);
	}

	UFUNCTION()
	static double Asinh(double D) {
		return std::asinh(D);
	}

	UFUNCTION()
	static double Atan2(double Y, double X) {
		return std::atan2(Y, X);
	}

	UFUNCTION()
	static double Atanh(double D) {
		return std::atanh(D);
	}

	UFUNCTION()
	static double Cbrt(double D) {
		return std::cbrt(D);
	}

	UFUNCTION()
	static double Cos(double D) {
		return std::cos(D);
	}

	UFUNCTION()
	static double Cosh(double Value) {
		return std::cosh(Value);
	}

	UFUNCTION()
	static double Exp(double D) {
		return std::exp(D);
	}

	UFUNCTION()
	static double Log(double A, double NewBase) {
		return std::log(A) / std::log(NewBase);
	}

	UFUNCTION()
	static double Log10(double D) {
		return std::log10(D);
	}

	UFUNCTION()
	static double Pow(double X, double Y) {
		return std::pow(X, Y);
	}

	UFUNCTION()
	static double Sin(double A) {
		return std::sin(A);
	}

	UFUNCTION()
	static double Sinh(double Value) {
		return std::sinh(Value);
	}

	UFUNCTION()
	static double Sqrt(double D) {
		return std::sqrt(D);
	}

	UFUNCTION()
	static double Tan(double A) {
		return std::tan(A);
	}

	UFUNCTION()
	static double Tanh(double Value) {
		return std::tanh(Value);
	}

private:

	static double FMod(double X, double Y) {
		return std::fmod(X, Y);
	}
};