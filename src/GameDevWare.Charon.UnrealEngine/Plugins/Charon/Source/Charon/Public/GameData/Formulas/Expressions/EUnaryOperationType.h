// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "CoreMinimal.h"

enum class EUnaryOperationType : uint8
{
	UnaryPlus,
	Negate,
	NegateChecked,
	Not,
	Complement,
	UncheckedScope,
	CheckedScope,
	Group
};