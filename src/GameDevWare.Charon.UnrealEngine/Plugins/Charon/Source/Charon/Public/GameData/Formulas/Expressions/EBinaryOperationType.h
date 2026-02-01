// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "CoreMinimal.h"

enum class EBinaryOperationType : uint8
{
	And,
	Or,
	ExclusiveOr,
	Multiply,
	MultiplyChecked,
	Divide,
	DivideChecked,
	Power,
	Modulo,
	Add,
	AddChecked,
	Subtract,
	SubtractChecked,
	LeftShift,
	RightShift,
	GreaterThan,
	GreaterThanOrEqual,
	LessThan,
	LessThanOrEqual,
	Equal,
	NotEqual,
	AndAlso,
	OrElse,
	Coalesce
};