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