// Copyright GameDevWare, Denis Zykov 2025

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
	NewArrayInitExpression,
	NewExpression,
	TypeIsExpression,
	TypeOfExpression,
	UnaryExpression,
	MemberInitExpression,
	MemberAssignmentBinding,
	MemberMemberBinding,
	MemberListBinding,
};