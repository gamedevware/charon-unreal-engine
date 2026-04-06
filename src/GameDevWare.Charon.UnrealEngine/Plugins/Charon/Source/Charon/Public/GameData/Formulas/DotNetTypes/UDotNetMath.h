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
	static double Truncate(double d) {
		return std::trunc(d);
	}

	UFUNCTION()
	static double IEEERemainder(double x, double y) {
		return std::remainder(x, y);
	}

	UFUNCTION()
	static double Floor(double d) {
		return std::floor(d);
	}

	UFUNCTION()
	static double Ceiling(double d) {
		return std::ceil(d);
	}

	UFUNCTION()
	static double Acos(double d) {
		return std::acos(d);
	}

	UFUNCTION()
	static double Acosh(double d) {
		return std::acosh(d);
	}

	UFUNCTION()
	static double Asin(double d) {
		return std::asin(d);
	}

	UFUNCTION()
	static double Asinh(double d) {
		return std::asinh(d);
	}

	UFUNCTION()
	static double Atan2(double y, double x) {
		return std::atan2(y, x);
	}

	UFUNCTION()
	static double Atanh(double d) {
		return std::atanh(d);
	}

	UFUNCTION()
	static double Cbrt(double d) {
		return std::cbrt(d);
	}

	UFUNCTION()
	static double Cos(double d) {
		return std::cos(d);
	}

	UFUNCTION()
	static double Cosh(double value) {
		return std::cosh(value);
	}

	UFUNCTION()
	static double Exp(double d) {
		return std::exp(d);
	}

	UFUNCTION()
	static double Log(double A, double NewBase) {
		return std::log(A) / std::log(NewBase);
	}

	UFUNCTION()
	static double Log10(double d) {
		return std::log10(d);
	}

	UFUNCTION()
	static double Pow(double x, double y) {
		return std::pow(x, y);
	}

	UFUNCTION()
	static double Sin(double a) {
		return std::sin(a);
	}

	UFUNCTION()
	static double Sinh(double value) {
		return std::sinh(value);
	}

	UFUNCTION()
	static double Sqrt(double d) {
		return std::sqrt(d);
	}

	UFUNCTION()
	static double Tan(double a) {
		return std::tan(a);
	}

	UFUNCTION()
	static double Tanh(double value) {
		return std::tanh(value);
	}

private:

	static double FMod(double x, double y) {
		return std::fmod(x, y);
	}
};