#include "GameData/Formulas/FFormulaVariableValue.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "UObject/UnrealType.h"
#include "UObject/TextProperty.h"
#include "UObject/EnumProperty.h"
#include "UObject/StrProperty.h"
#include "UObject/FieldPathProperty.h"
#include "FFormulaVariableValueStruct.h"

DEFINE_LOG_CATEGORY(LogFormulaVariableValue);

FProperty* GetStructProperty(FName Name)
{
	UScriptStruct* Type = FFormulaVariableValueStruct::StaticStruct();
	FProperty* Property = Type->FindPropertyByName(Name);

	check(Property != nullptr);

	return Property;
}

FFormulaVariableValue::FFormulaVariableValue()
{
	this->Value.Set<nullptr_t>(nullptr);
	this->Type = GetStructProperty(TEXT("ObjectPtrValue"));
}

FFormulaVariableValue::FFormulaVariableValue(FProperty* FieldType, const void* ValuePtr)
{
	this->Type = FieldType;
	if (const FBoolProperty* BoolProp = CastField<FBoolProperty>(FieldType))
	{
		const bool BoolValue = BoolProp->GetPropertyValue(ValuePtr);
		this->Value.Set<bool>(BoolValue);
	}
	else if (const FNumericProperty* NumericProp = CastField<FNumericProperty>(FieldType))
	{
		if (NumericProp->IsFloatingPoint())
		{
			if (NumericProp->GetElementSize() <= 4)
			{
				const float SingleValue = NumericProp->GetFloatingPointPropertyValue(ValuePtr);
				this->Value.Set<float>(SingleValue);
			}
			else
			{
				const double DoubleValue = NumericProp->GetFloatingPointPropertyValue(ValuePtr);
				this->Value.Set<double>(DoubleValue);
			}
		}
		else if (NumericProp->CanHoldValue(-1))
		{
			const int64 Int64Value = NumericProp->GetSignedIntPropertyValue(ValuePtr);
			switch (NumericProp->GetElementSize())
			{
			case 1:
				this->Value.Set<int32>(static_cast<int8>(Int64Value));
				break;
			case 2:
				this->Value.Set<int32>(static_cast<int16>(Int64Value));
				break;
			case 4:
				this->Value.Set<int32>(static_cast<int32>(Int64Value));
				break;
			default:
				this->Value.Set<int64>(Int64Value);
				break;
			}
		}
		else
		{
			const uint64 UInt64Value = NumericProp->GetUnsignedIntPropertyValue(ValuePtr);
			switch (NumericProp->GetElementSize())
			{
			case 1:
				this->Value.Set<uint32>(static_cast<uint8>(UInt64Value));
				break;
			case 2:
				this->Value.Set<uint32>(static_cast<uint16>(UInt64Value));
				break;
			case 4:
				this->Value.Set<uint32>(static_cast<uint32>(UInt64Value));
				break;
			default:
				this->Value.Set<uint64>(UInt64Value);
				break;
			}
		}
	}
	else if (const FObjectPropertyBase* ObjectProp = CastField<FObjectPropertyBase>(FieldType))
	{
		if (UObject* ObjectValue = ObjectProp->GetObjectPropertyValue(ValuePtr))
		{
			this->Value.Set<UObject*>(ObjectValue);
		}
		else
		{
			this->Value.Set<nullptr_t>(nullptr);
		}
	}
	else if (const FStrProperty* StringProp = CastField<FStrProperty>(FieldType))
	{
		const FString StringValue = StringProp->GetPropertyValue(ValuePtr);
		this->Value.Set<FString>(StringValue);
	}
	else if (FTextProperty* TextProp = CastField<FTextProperty>(FieldType))
	{
		const FText TextValue = TextProp->GetPropertyValue(ValuePtr);
		this->Value.Set<FText>(TextValue);
	}
	else if (FNameProperty* NameProp = CastField<FNameProperty>(FieldType))
	{
		const FName NameValue = NameProp->GetPropertyValue(ValuePtr);
		this->Value.Set<FName>(NameValue);
	}
	else
	{
		FByteArray Buffer;
		Buffer.AddZeroed(FieldType->GetElementSize());
		FieldType->CopyCompleteValue(Buffer.GetData(), ValuePtr);
		this->Value.Set<FByteArray>(Buffer);
	}
}

FFormulaVariableValue FFormulaVariableValue::Create(const bool Value)
{
	FFormulaVariableValue VariableValue = FFormulaVariableValue();
	VariableValue.Value.Set<bool>(Value);
	VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, BoolValue));
	return VariableValue;
}

FFormulaVariableValue FFormulaVariableValue::Create(const int8 Value)
{
	FFormulaVariableValue VariableValue = FFormulaVariableValue();
	VariableValue.Value.Set<int32>(Value);
	VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, Int32Value));
	return VariableValue;
}

FFormulaVariableValue FFormulaVariableValue::Create(const int16 Value)
{
	FFormulaVariableValue VariableValue = FFormulaVariableValue();
	VariableValue.Value.Set<int32>(Value);
	VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, Int32Value));
	return VariableValue;
}

FFormulaVariableValue FFormulaVariableValue::Create(const int32 Value)
{
	FFormulaVariableValue VariableValue = FFormulaVariableValue();
	VariableValue.Value.Set<int32>(Value);
	VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, Int32Value));
	return VariableValue;
}

FFormulaVariableValue FFormulaVariableValue::Create(const int64 Value)
{
	FFormulaVariableValue VariableValue = FFormulaVariableValue();
	VariableValue.Value.Set<int64>(Value);
	VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, Int64Value));
	return VariableValue;
}

FFormulaVariableValue FFormulaVariableValue::Create(const uint8 Value)
{
	FFormulaVariableValue VariableValue = FFormulaVariableValue();
	VariableValue.Value.Set<uint32>(Value);
	VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, UInt32Value));
	return VariableValue;
}

FFormulaVariableValue FFormulaVariableValue::Create(const uint16 Value)
{
	FFormulaVariableValue VariableValue = FFormulaVariableValue();
	VariableValue.Value.Set<uint32>(Value);
	VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, UInt32Value));
	return VariableValue;
}

FFormulaVariableValue FFormulaVariableValue::Create(const uint32 Value)
{
	FFormulaVariableValue VariableValue = FFormulaVariableValue();
	VariableValue.Value.Set<uint32>(Value);
	VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, UInt32Value));
	return VariableValue;
}

FFormulaVariableValue FFormulaVariableValue::Create(const uint64 Value)
{
	FFormulaVariableValue VariableValue = FFormulaVariableValue();
	VariableValue.Value.Set<uint64>(Value);
	VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, UInt64Value));
	return VariableValue;
}

FFormulaVariableValue FFormulaVariableValue::Create(const float Value)
{
	FFormulaVariableValue VariableValue = FFormulaVariableValue();
	VariableValue.Value.Set<float>(Value);
	VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, SingleValue));
	return VariableValue;
}

FFormulaVariableValue FFormulaVariableValue::Create(const double Value)
{
	FFormulaVariableValue VariableValue = FFormulaVariableValue();
	VariableValue.Value.Set<double>(Value);
	VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, DoubleValue));
	return VariableValue;
}

FFormulaVariableValue FFormulaVariableValue::Create(const FString& Value)
{
	FFormulaVariableValue VariableValue = FFormulaVariableValue();
	VariableValue.Value.Set<FString>(Value);
	VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, FStringValue));
	return VariableValue;
}

FFormulaVariableValue FFormulaVariableValue::Create(const FName Value)
{
	FFormulaVariableValue VariableValue = FFormulaVariableValue();
	VariableValue.Value.Set<FName>(Value);
	VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, FNameValue));
	return VariableValue;
}

FFormulaVariableValue FFormulaVariableValue::Create(const FText& Value)
{
	FFormulaVariableValue VariableValue = FFormulaVariableValue();
	VariableValue.Value.Set<FText>(Value);
	VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, FTextValue));
	return VariableValue;
}

FFormulaVariableValue FFormulaVariableValue::Create(const TObjectPtr<UObject> Value)
{
	FFormulaVariableValue VariableValue = FFormulaVariableValue();
	VariableValue.Value.Set<UObject*>(Value.Get());
	VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, ObjectPtrValue));
	return VariableValue;
}

FFormulaVariableValue FFormulaVariableValue::Create(UObject* Value)
{
	FFormulaVariableValue VariableValue = FFormulaVariableValue();
	if (Value)
	{
		VariableValue.Value.Set<UObject*>(Value);
		VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, ObjectPtrValue));
	}
	return VariableValue;
}

bool FFormulaVariableValue::TryCopyCompleteValue(const FProperty* OutType, void* OutValue) const
{
	if (const FOptionalProperty* OptionalProp = CastField<FOptionalProperty>(OutType))
	{
		if (this->Value.IsType<nullptr_t>())
		{
			OptionalProp->MarkUnset(OutValue);
			return true;
		}
		else
		{
			if (OptionalProp->IsSet(OutValue))
			{
				return  TryCopyCompleteValue(OptionalProp->GetValueProperty(), OptionalProp->GetValuePointerForReplace(OutValue));
			}
			else
			{
				
				bool bCopySuccess = TryCopyCompleteValue(OptionalProp->GetValueProperty(), OptionalProp->MarkSetAndGetInitializedValuePointerToReplace(OutValue));
				if (!bCopySuccess)
				{
					OptionalProp->MarkUnset(OutValue); // return back to unset state
				}
				return bCopySuccess;
			}
		}
	}
	if (const FObjectPropertyBase* OutObjectProp = CastField<FObjectPropertyBase>(OutType))
	{
		if (this->Value.IsType<nullptr_t>())
		{
			*static_cast<nullptr_t*>(OutValue) = nullptr;
			return true;
		}
		else if (const FObjectPropertyBase* ThisObjectProp = CastField<FObjectPropertyBase>(this->Type))
		{
			if (ThisObjectProp->PropertyClass.IsA(OutObjectProp->PropertyClass))
			{
				CopyCompleteValue(OutValue);
				return true;
			}
		}
	}

	if (this->Type->SameType(OutType))
	{
		CopyCompleteValue(OutValue);
		return true;
	}
	return false;
}

void FFormulaVariableValue::CopyCompleteValue(void* DestinationPtr) const
{
	Visit([this, DestinationPtr]<typename ValueType>(ValueType&& InValue)
	{
		using T = std::decay_t<ValueType>;

		if constexpr (std::is_same_v<T, FByteArray>)
		{
			this->Type->CopyCompleteValue(DestinationPtr, InValue.GetData());
		}
		else if constexpr (std::is_same_v<T, nullptr_t>)
		{
			// For null pointers, we pass the address of a null variable
			void* NullValue = nullptr;
			this->Type->CopyCompleteValue(DestinationPtr, &NullValue);
		}
		else
		{
			// This handles all standard types (bool, int32, FString, etc.)
			this->Type->CopyCompleteValue(DestinationPtr, &InValue);
		}
	}, Value);
}

FString FFormulaVariableValue::ToString() const
{
	if (this->Value.IsType<nullptr_t>())
	{
		return TEXT("null");
	}
	else if (this->Value.IsType<bool>())
	{
		return this->Value.Get<bool>() ? TEXT("true") : TEXT("false");
	}
	else if (this->Value.IsType<int32>())
	{
		return FString::FromInt(this->Value.Get<int32>());
	}
	else if (this->Value.IsType<int64>())
	{
		return FString::Printf(TEXT("%lld"), this->Value.Get<int64>());
	}
	else if (this->Value.IsType<uint32>())
	{
		return FString::Printf(TEXT("%u"), this->Value.Get<uint32>());
	}
	else if (this->Value.IsType<uint64>())
	{
		return FString::Printf(TEXT("%llu"), this->Value.Get<uint64>());
	}
	else if (this->Value.IsType<float>())
	{
		return FString::SanitizeFloat(this->Value.Get<float>());
	}
	else if (this->Value.IsType<double>())
	{
		return FString::Printf(TEXT("%f"), this->Value.Get<double>());
	}
	else if (this->Value.IsType<FString>())
	{
		return this->Value.Get<FString>();
	}
	else if (this->Value.IsType<FText>())
	{
		return this->Value.Get<FText>().ToString();
	}
	else if (this->Value.IsType<FName>())
	{
		return this->Value.Get<FName>().ToString();
	}
	else if (this->Value.IsType<UObject*>())
	{
		if (const UObject* ObjectPtr = this->Value.Get<UObject*>())
		{
			return ObjectPtr->GetFullName();
		}
		return TEXT("null");
	}
	else
	{
		return FString::Printf(TEXT("Unsupported Type: %s"), *this->GetFieldType()->GetCPPType());
	}
}
