#pragma once

#include "CoreMinimal.h"

enum class EFormulaExpressionType : uint8
{
	BinaryExpression,
	ConditionExpression,
	ConstantExpression,
	ConvertExpression,
	DefaultExpression,
	IndexExpression,
	InvokeExpression,
	LambdaExpression,
	MemberExpression,
	NewArrayBoundExpression,
	NewExpression,
	TypeIsExpression,
	TypeOfExpression,
	UnaryExpression
};