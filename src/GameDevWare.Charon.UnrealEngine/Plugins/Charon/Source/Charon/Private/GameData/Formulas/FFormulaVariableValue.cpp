#include "GameData/Formulas/FFormulaVariableValue.h"

FFormulaVariableValue::FFormulaVariableValue()
{
	this->Value.Set<nullptr_t>(nullptr);
	this->Type = EFormulaVariableType::Null;
}

FFormulaVariableValue FFormulaVariableValue::Create(const bool Value)
{
	FFormulaVariableValue VariableValue = FFormulaVariableValue();
	VariableValue.Set<bool>(Value, EFormulaVariableType::Bool);
	return VariableValue;
}

FFormulaVariableValue FFormulaVariableValue::Create(const int8 Value)
{
	FFormulaVariableValue VariableValue = FFormulaVariableValue();
	VariableValue.Set<int32>((int32)Value, EFormulaVariableType::Int32);
	return VariableValue;
}

FFormulaVariableValue FFormulaVariableValue::Create(const int16 Value)
{
	FFormulaVariableValue VariableValue = FFormulaVariableValue();
	VariableValue.Set<int32>((int32)Value, EFormulaVariableType::Int32);
	return VariableValue;
}

FFormulaVariableValue FFormulaVariableValue::Create(const int32 Value)
{
	FFormulaVariableValue VariableValue = FFormulaVariableValue();
	VariableValue.Set<int32>(Value, EFormulaVariableType::Int32);
	return VariableValue;
}

FFormulaVariableValue FFormulaVariableValue::Create(const int64 Value)
{
	FFormulaVariableValue VariableValue = FFormulaVariableValue();
	VariableValue.Set<int64>(Value, EFormulaVariableType::Int64);
	return VariableValue;
}

FFormulaVariableValue FFormulaVariableValue::Create(const uint8 Value)
{
	FFormulaVariableValue VariableValue = FFormulaVariableValue();
	VariableValue.Set<uint32>((uint32)Value, EFormulaVariableType::UInt32);
	return VariableValue;
}

FFormulaVariableValue FFormulaVariableValue::Create(const uint16 Value)
{
	FFormulaVariableValue VariableValue = FFormulaVariableValue();
	VariableValue.Set<uint32>((uint32)Value, EFormulaVariableType::UInt32);
	return VariableValue;
}

FFormulaVariableValue FFormulaVariableValue::Create(const uint32 Value)
{
	FFormulaVariableValue VariableValue = FFormulaVariableValue();
	VariableValue.Set<uint32>(Value, EFormulaVariableType::UInt32);
	return VariableValue;
}

FFormulaVariableValue FFormulaVariableValue::Create(const uint64 Value)
{
	FFormulaVariableValue VariableValue = FFormulaVariableValue();
	VariableValue.Set<uint64>(Value, EFormulaVariableType::UInt64);
	return VariableValue;
}

FFormulaVariableValue FFormulaVariableValue::Create(const float Value)
{
	FFormulaVariableValue VariableValue = FFormulaVariableValue();
	VariableValue.Set<float>(Value, EFormulaVariableType::Float);
	return VariableValue;
}

FFormulaVariableValue FFormulaVariableValue::Create(const double Value)
{
	FFormulaVariableValue VariableValue = FFormulaVariableValue();
	VariableValue.Set<double>(Value, EFormulaVariableType::Double);
	return VariableValue;
}

FFormulaVariableValue FFormulaVariableValue::Create(const FString& Value)
{
	FFormulaVariableValue VariableValue = FFormulaVariableValue();
	VariableValue.Set<FString>(Value, EFormulaVariableType::FString);
	return VariableValue;
}

FFormulaVariableValue FFormulaVariableValue::Create(const FName Value)
{
	FFormulaVariableValue VariableValue = FFormulaVariableValue();
	VariableValue.Set<FName>(Value, EFormulaVariableType::FName);
	return VariableValue;
}

FFormulaVariableValue FFormulaVariableValue::Create(const FText& Value)
{
	FFormulaVariableValue VariableValue = FFormulaVariableValue();
	VariableValue.Set<FText>(Value, EFormulaVariableType::FText);
	return VariableValue;
}

FFormulaVariableValue FFormulaVariableValue::Create(const TObjectPtr<UObject> Value)
{
	FFormulaVariableValue VariableValue = FFormulaVariableValue();
	VariableValue.Set<UObject*>(Value.Get(), EFormulaVariableType::UObjectPointer);
	return VariableValue;
}

FFormulaVariableValue FFormulaVariableValue::Create(UObject* Value)
{
	FFormulaVariableValue VariableValue = FFormulaVariableValue();
	if (Value)
	{
		VariableValue.Set<UObject*>(Value, EFormulaVariableType::UObjectPointer);
	}
	return VariableValue;
}

bool FFormulaVariableValue::TryGet(bool& OutValue) const
{
	switch (this->Type)
	{
	case EFormulaVariableType::Bool:
		OutValue = this->Value.Get<bool>();
		return true;
	case EFormulaVariableType::Int32: OutValue = this->Value.Get<int32>() != 0;
		return true;
	case EFormulaVariableType::Int64: OutValue = this->Value.Get<int64>() != 0;
		return true;
	case EFormulaVariableType::UInt32: OutValue = this->Value.Get<uint32>() != 0;
		return true;
	case EFormulaVariableType::UInt64: OutValue = this->Value.Get<uint64>() != 0;
		return true;
	case EFormulaVariableType::Float: OutValue = this->Value.Get<float>() != 0;
		return true;
	case EFormulaVariableType::Double: OutValue = this->Value.Get<double>() != 0;
		return true;
	case EFormulaVariableType::FString:
		{
			FString BoolString = this->Value.Get<FString>().ToLower();
			if (BoolString == TEXT("true"))
			{
				OutValue = true;
				return true;
			}
			else if (BoolString == TEXT("false"))
			{
				OutValue = false;
				return true;
			}
			return  false;
		}
	default:
		return false;
	}
}

FString FFormulaVariableValue::ToString() const
{
	switch (this->Type)
	{
	case EFormulaVariableType::Null: return TEXT("null");
	case EFormulaVariableType::Bool: return this->Value.Get<bool>() ? TEXT("true") : TEXT("false");
	case EFormulaVariableType::Int32: return FString::FromInt(this->Value.Get<int32>());
	case EFormulaVariableType::Int64: return FString::Printf(TEXT("%lld"), this->Value.Get<int64>());
	case EFormulaVariableType::UInt32: return FString::Printf(TEXT("%u"), this->Value.Get<uint32>());
	case EFormulaVariableType::UInt64: return FString::Printf(TEXT("%llu"), this->Value.Get<uint64>());
	case EFormulaVariableType::Float: return FString::SanitizeFloat(this->Value.Get<float>());
	case EFormulaVariableType::Double: return FString::Printf(TEXT("%f"), this->Value.Get<double>());
	case EFormulaVariableType::FString: return this->Value.Get<FString>();
	case EFormulaVariableType::FText: return this->Value.Get<FText>().ToString();
	case EFormulaVariableType::FName: return this->Value.Get<FName>().ToString();
	case EFormulaVariableType::UObjectPointer: return this->Value.Get<UObject*>()->GetFullName();
	default: return TEXT("Unknown");
	}
}
