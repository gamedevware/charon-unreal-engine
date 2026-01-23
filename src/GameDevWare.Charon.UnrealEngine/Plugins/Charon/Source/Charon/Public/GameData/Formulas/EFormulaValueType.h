#pragma once

#include "UObject/UnrealType.h"

enum class EFormulaValueType
{
	Null,
	Boolean,
	UInt8,
	UInt16,
	UInt32,
	UInt64,
	Int8,
	Int16,
	Int32,
	Int64,
	Float,
	Double,
	Timespan,
	DateTime,
	String,
	Name,
	Text,
	ObjectPtr,
	Struct,
	Enum,
};