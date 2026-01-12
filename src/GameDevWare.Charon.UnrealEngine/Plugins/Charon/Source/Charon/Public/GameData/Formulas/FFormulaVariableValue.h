#pragma once

using FFormulaVariant = TVariant<
	bool,
	uint32,
	uint64,
	int32,
	int64,
	float,
	double,
	FString,
	FName,
	FText,
	UObject*,
	/*
		TObjectPtr<UObject>,
		TSubclassOf<UObject>,
		TSoftObjectPtr<>,
		TSoftClassPtr<>,
		FLazyObjectPtr,
		TWeakObjectPtr<>,
	
		FVector,
		FVector2D,
		FRotator,
		FTransform,
		FColor,
		FLinearColor,
		FTimespan,
		FScriptInterface,
		FFieldPath,
	*/
	nullptr_t
>;

class CHARON_API FFormulaVariableValue
{
private:
	FFormulaVariant Value;

public:
	EPropertyType Type;

	FFormulaVariableValue();

	static FFormulaVariableValue Create(const bool Value)
	{
		FFormulaVariableValue VariableValue = FFormulaVariableValue();
		VariableValue.Set<bool>(Value, EPropertyType::CPT_Bool);
		return VariableValue;
	}
	static FFormulaVariableValue Create(const int8 Value)
	{
		FFormulaVariableValue VariableValue = FFormulaVariableValue();
		VariableValue.Set<int32>((int32)Value, EPropertyType::CPT_Int);
		return VariableValue;
	}
	static FFormulaVariableValue Create(const int16 Value)
	{
		FFormulaVariableValue VariableValue = FFormulaVariableValue();
		VariableValue.Set<int32>((int32)Value, EPropertyType::CPT_Int);
		return VariableValue;
	}
	static FFormulaVariableValue Create(const int32 Value)
	{
		FFormulaVariableValue VariableValue = FFormulaVariableValue();
		VariableValue.Set<int32>(Value, EPropertyType::CPT_Int);
		return VariableValue;
	}
	static FFormulaVariableValue Create(const int64 Value)
	{
		FFormulaVariableValue VariableValue = FFormulaVariableValue();
		VariableValue.Set<int64>(Value, EPropertyType::CPT_Int64);
		return VariableValue;
	}
	static FFormulaVariableValue Create(const uint8 Value)
	{
		FFormulaVariableValue VariableValue = FFormulaVariableValue();
		VariableValue.Set<uint32>((uint32)Value, EPropertyType::CPT_UInt32);
		return VariableValue;
	}
	static FFormulaVariableValue Create(const uint16 Value)
	{
		FFormulaVariableValue VariableValue = FFormulaVariableValue();
		VariableValue.Set<uint32>((uint32)Value, EPropertyType::CPT_UInt32);
		return VariableValue;
	}
	static FFormulaVariableValue Create(const uint32 Value)
	{
		FFormulaVariableValue VariableValue = FFormulaVariableValue();
		VariableValue.Set<uint32>(Value, EPropertyType::CPT_UInt32);
		return VariableValue;
	}
	static FFormulaVariableValue Create(const uint64 Value)
	{
		FFormulaVariableValue VariableValue = FFormulaVariableValue();
		VariableValue.Set<uint64>(Value, EPropertyType::CPT_UInt64);
		return VariableValue;
	}
	static FFormulaVariableValue Create(const float Value)
	{
		FFormulaVariableValue VariableValue = FFormulaVariableValue();
		VariableValue.Set<float>(Value, EPropertyType::CPT_Float);
		return VariableValue;
	}
	static FFormulaVariableValue Create(const double Value)
	{
		FFormulaVariableValue VariableValue = FFormulaVariableValue();
		VariableValue.Set<double>(Value, EPropertyType::CPT_Double);
		return VariableValue;
	}
	static FFormulaVariableValue Create(const FString& Value)
	{
		FFormulaVariableValue VariableValue = FFormulaVariableValue();
		VariableValue.Set<FString>(Value, EPropertyType::CPT_String);
		return VariableValue;
	}
	static FFormulaVariableValue Create(const FName Value)
	{
		FFormulaVariableValue VariableValue = FFormulaVariableValue();
		VariableValue.Set<FName>(Value, EPropertyType::CPT_Name);
		return VariableValue;
	}
	static FFormulaVariableValue Create(const FText& Value)
	{
		FFormulaVariableValue VariableValue = FFormulaVariableValue();
		VariableValue.Set<FText>(Value, EPropertyType::CPT_Text);
		return VariableValue;
	}
	static FFormulaVariableValue Create(const TObjectPtr<UObject> Value)
	{
		FFormulaVariableValue VariableValue = FFormulaVariableValue();
		VariableValue.Set<UObject*>(Value.Get(), EPropertyType::CPT_ObjectReference);
		return VariableValue;
	}
	static FFormulaVariableValue Create(UObject* Value)
	{
		FFormulaVariableValue VariableValue = FFormulaVariableValue();
		if (Value)
		{
			VariableValue.Set<UObject*>(Value, EPropertyType::CPT_ObjectReference);
		}
		return VariableValue;
	}

	bool TryGet(bool& OutValue) const
	{
		switch (this->Type)
		{
		case EPropertyType::CPT_Bool:
			OutValue = this->Value.Get<bool>();
			return true;
		case EPropertyType::CPT_Int: OutValue = this->Value.Get<int32>() != 0;
			return true;
		case EPropertyType::CPT_Int64: OutValue = this->Value.Get<int64>() != 0;
			return true;
		case EPropertyType::CPT_UInt32: OutValue = this->Value.Get<uint32>() != 0;
			return true;
		case EPropertyType::CPT_UInt64: OutValue = this->Value.Get<uint64>() != 0;
			return true;
		case EPropertyType::CPT_Float: OutValue = this->Value.Get<float>() != 0;
			return true;
		case EPropertyType::CPT_Double: OutValue = this->Value.Get<double>() != 0;
			return true;
		default:
			return false;
		}
	}

	FString ToString() const
	{
		switch (this->Type)
		{
		case EPropertyType::CPT_None: return TEXT("null");
		case EPropertyType::CPT_Bool: return this->Value.Get<bool>() ? TEXT("true") : TEXT("false");
		case EPropertyType::CPT_Int: return FString::FromInt(this->Value.Get<int32>());
		case EPropertyType::CPT_Int64: return FString::Printf(TEXT("%lld"), this->Value.Get<int64>());
		case EPropertyType::CPT_UInt32: return FString::Printf(TEXT("%u"), this->Value.Get<uint32>());
		case EPropertyType::CPT_UInt64: return FString::Printf(TEXT("%llu"), this->Value.Get<uint64>());
		case EPropertyType::CPT_Float: return FString::SanitizeFloat(this->Value.Get<float>());
		case EPropertyType::CPT_Double: return FString::Printf(TEXT("%f"), this->Value.Get<double>());
		case EPropertyType::CPT_String: return this->Value.Get<FString>();
		case EPropertyType::CPT_Text: return this->Value.Get<FText>().ToString();
		case EPropertyType::CPT_Name: return this->Value.Get<FName>().ToString();
		case EPropertyType::CPT_ObjectReference: return this->Value.Get<UObject*>()->GetFullName();
		default: return TEXT("Unknown");
		}
	}

private:
	template <typename U>
	void Set(typename TIdentity<U>::Type&& Value, const EPropertyType Type)
	{
		this->Value.Set<U>(Value);
		this->Type = Type;
	}

	/** Set a specifically-typed value into the variant */
	template <typename U>
	void Set(const typename TIdentity<U>::Type& Value, const EPropertyType Type)
	{
		this->Value.Set<U>(Value);
		this->Type = Type;
	}
};
