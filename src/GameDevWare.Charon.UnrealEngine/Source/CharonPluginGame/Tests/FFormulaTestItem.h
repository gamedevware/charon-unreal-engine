// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "FFormulaTestItem.generated.h"

/**
 * A small struct with no reference back to UFormulaTestObject, so it can be used as the
 * element type of an array property on that object. Exists to cover the element kinds
 * FNewExpression::TryCreateArray cannot synthesize, which element-preserving LINQ
 * operators must still support by reusing the source array property.
 */
USTRUCT()
struct FFormulaTestItem
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Value = 0;

	UPROPERTY()
	FString Name;
};
