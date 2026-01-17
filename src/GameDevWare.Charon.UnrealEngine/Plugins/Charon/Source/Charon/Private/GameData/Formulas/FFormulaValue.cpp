#include "utility"
#include "GameData/Formulas/FFormulaValue.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "UObject/UnrealType.h"
#include "UObject/TextProperty.h"
#include "UObject/EnumProperty.h"
#if __has_include("UObject/StrProperty.h")
#include "UObject/StrProperty.h"
#endif
#include "UObject/FieldPathProperty.h"
#include "FFormulaVariableValueStruct.h"
#include "Misc/EngineVersionComparison.h"

DEFINE_LOG_CATEGORY(LogFormulaVariableValue);

FProperty* GetStructProperty(FName Name)
{
	UScriptStruct* Type = FFormulaVariableValueStruct::StaticStruct();
	FProperty* Property = Type->FindPropertyByName(Name);

	check(Property != nullptr);

	return Property;
}

EFormulaValueType FFormulaValue::GetPropertyTypeCode(const FProperty* FieldType)
{
#if UE_VERSION_NEWER_THAN(5, 5, -1)
	const int32 ElementSize = FieldType->GetElementSize();
#else
	const int32 ElementSize = FieldType->ElementSize;
#endif
	if (CastField<FBoolProperty>(FieldType))
	{
		return EFormulaValueType::Boolean;
	}
	else if (const FNumericProperty* NumericProp = CastField<FNumericProperty>(FieldType))
	{
		if (NumericProp->IsFloatingPoint())
		{
			return ElementSize <= 4 ? EFormulaValueType::Float : EFormulaValueType::Double;
		}
		else if (NumericProp->CanHoldValue(-1))
		{
			switch (ElementSize)
			{
			case 1:
				return EFormulaValueType::Int8;
			case 2:
				return EFormulaValueType::Int16;
			case 4:
				return EFormulaValueType::Int32;
			default:
				return EFormulaValueType::Int64;
			}
		}
		else
		{
			switch (ElementSize)
			{
			case 1:
				return EFormulaValueType::UInt8;
			case 2:
				return EFormulaValueType::UInt16;
			case 4:
				return EFormulaValueType::UInt32;
			default:
				return EFormulaValueType::UInt64;
			}
		}
	}
	else if (CastField<FObjectPropertyBase>(FieldType))
	{
		const auto NullProperty = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, NullPtrValue));
		if (FieldType == NullProperty)
		{
			return EFormulaValueType::Null;
		}
		else
		{
			return EFormulaValueType::ObjectPtr;
		}
	}
	else if (const FStructProperty* StructProp = CastField<FStructProperty>(FieldType))
	{
		const auto TimespanStruct = CastField<FStructProperty>(
			GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, FTimespanValue)))->Struct;
		const auto DateTimeStruct = CastField<FStructProperty>(
			GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, FDateTimeValue)))->Struct;

		if (StructProp->Struct == TimespanStruct)
		{
			return EFormulaValueType::Timespan;
		}
		else if (StructProp->Struct == DateTimeStruct)
		{
			return EFormulaValueType::DateTime;
		}
		else
		{
			return EFormulaValueType::Struct;
		}
	}
	else if (CastField<FEnumProperty>(FieldType))
	{
		return EFormulaValueType::Enum;
	}
	else if (CastField<FStrProperty>(FieldType))
	{
		return EFormulaValueType::String;
	}
	else if (CastField<FTextProperty>(FieldType))
	{
		return EFormulaValueType::Text;
	}
	else if (CastField<FNameProperty>(FieldType))
	{
		return EFormulaValueType::Name;
	}
#if UE_VERSION_NEWER_THAN(5, 6, -1)
	else if (CastField<FOptionalProperty>(FieldType))
	{
		return EFormulaValueType::Optional;
	}
#endif
	else
	{
		return EFormulaValueType::Struct;
	}
}

FFormulaValue::FFormulaValue()
{
	this->Value.Set<nullptr_t>(nullptr);
	this->Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, NullPtrValue));
	this->TypeCode = EFormulaValueType::Null;
}

FFormulaValue::FFormulaValue(FProperty* FieldType, const void* ValuePtr)
{
	this->Type = FieldType;
#if UE_VERSION_NEWER_THAN(5, 5, -1)
	const int32 ElementSize = FieldType->GetElementSize();
#else
	const int32 ElementSize = FieldType->ElementSize;
#endif

	switch (GetPropertyTypeCode(FieldType))
	{
	case EFormulaValueType::Null:
		this->Value.Set<nullptr_t>(nullptr);
		this->Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, NullPtrValue));
		break;
	case EFormulaValueType::Boolean:
		this->Value.Set<bool>(*static_cast<const bool*>(ValuePtr));
		break;
	case EFormulaValueType::UInt8:
		this->Value.Set<uint32>(*static_cast<const uint8*>(ValuePtr));
		break;
	case EFormulaValueType::UInt16:
		this->Value.Set<uint32>(*static_cast<const uint16*>(ValuePtr));
		break;
	case EFormulaValueType::UInt32:
		this->Value.Set<uint32>(*static_cast<const uint32*>(ValuePtr));
		break;
	case EFormulaValueType::UInt64:
		this->Value.Set<uint64>(*static_cast<const uint64*>(ValuePtr));
		break;
	case EFormulaValueType::Int8:
		this->Value.Set<int32>(*static_cast<const int8*>(ValuePtr));
		break;
	case EFormulaValueType::Int16:
		this->Value.Set<int32>(*static_cast<const int16*>(ValuePtr));
		break;
	case EFormulaValueType::Int32:
		this->Value.Set<int32>(*static_cast<const int32*>(ValuePtr));
		break;
	case EFormulaValueType::Int64:
		this->Value.Set<int64>(*static_cast<const int64*>(ValuePtr));
		break;
	case EFormulaValueType::Float:
		this->Value.Set<float>(*static_cast<const float*>(ValuePtr));
		break;
	case EFormulaValueType::Double:
		this->Value.Set<double>(*static_cast<const double*>(ValuePtr));
		break;
	case EFormulaValueType::Timespan:
		this->Value.Set<FTimespan>(*static_cast<const FTimespan*>(ValuePtr));
		break;
	case EFormulaValueType::DateTime:
		this->Value.Set<FDateTime>(*static_cast<const FDateTime*>(ValuePtr));
		break;
	case EFormulaValueType::String:
		if (const FStrProperty* StringProp = CastField<FStrProperty>(FieldType))
		{
			this->Value.Set<FString>(StringProp->GetPropertyValue(ValuePtr));
		}
		else { checkNoEntry(); }
		break;
	case EFormulaValueType::Name:
		if (FNameProperty* NameProp = CastField<FNameProperty>(FieldType))
		{
			const FName NameValue = NameProp->GetPropertyValue(ValuePtr);
			this->Value.Set<FName>(NameValue);
		}
		else { checkNoEntry(); }
		break;
	case EFormulaValueType::Text:
		if (FTextProperty* TextProp = CastField<FTextProperty>(FieldType))
		{
			const FText TextValue = TextProp->GetPropertyValue(ValuePtr);
			this->Value.Set<FText>(TextValue);
		}
		else { checkNoEntry(); }
		break;
	case EFormulaValueType::ObjectPtr:
		if (const FObjectPropertyBase* ObjectProp = CastField<FObjectPropertyBase>(FieldType))
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
		else { checkNoEntry(); }
		break;

	case EFormulaValueType::Optional:
#if UE_VERSION_NEWER_THAN(5, 6, -1)
		if (FOptionalProperty* OptProp = CastField<FOptionalProperty>(FieldType))
		{
			// optional types are erased during formula evaluation
			// and replaced with actual type - null or value 

			if (OptProp->IsSet(ValuePtr))
			{
				const auto OptionalValue = FFormulaValue(OptProp->GetValueProperty(),
				                                         OptProp->GetValuePointerForRead(ValuePtr));
				this->Type = OptionalValue.Type;
				this->Value = OptionalValue.Value;
			}
			else
			{
				this->Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, NullPtrValue));
				this->Value.Set<nullptr_t>(nullptr);
			}
		}
		else { checkNoEntry(); }
		break;
#else
		checkNoEntry();
		break;
#endif
	case EFormulaValueType::Enum:
	case EFormulaValueType::Struct:
	default:
		FByteArray Buffer;
		Buffer.AddZeroed(ElementSize);
		FieldType->CopyCompleteValue(Buffer.GetData(), ValuePtr);
		this->Value.Set<FByteArray>(Buffer);
		break;
	}
	this->TypeCode = GetPropertyTypeCode(this->Type);
}

FFormulaValue FFormulaValue::Create(const bool Value)
{
	FFormulaValue VariableValue = FFormulaValue();
	VariableValue.Value.Set<bool>(Value);
	VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, BoolValue));
	VariableValue.TypeCode = EFormulaValueType::Boolean;
	return VariableValue;
}

FFormulaValue FFormulaValue::Create(const int8 Value)
{
	FFormulaValue VariableValue = FFormulaValue();
	VariableValue.Value.Set<int32>(Value);
	VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, Int32Value));
	VariableValue.TypeCode = EFormulaValueType::Int32;
	return VariableValue;
}

FFormulaValue FFormulaValue::Create(const int16 Value)
{
	FFormulaValue VariableValue = FFormulaValue();
	VariableValue.Value.Set<int32>(Value);
	VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, Int32Value));
	VariableValue.TypeCode = EFormulaValueType::Int32;
	return VariableValue;
}

FFormulaValue FFormulaValue::Create(const int32 Value)
{
	FFormulaValue VariableValue = FFormulaValue();
	VariableValue.Value.Set<int32>(Value);
	VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, Int32Value));
	VariableValue.TypeCode = EFormulaValueType::Int32;
	return VariableValue;
}

FFormulaValue FFormulaValue::Create(const int64 Value)
{
	FFormulaValue VariableValue = FFormulaValue();
	VariableValue.Value.Set<int64>(Value);
	VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, Int64Value));
	VariableValue.TypeCode = EFormulaValueType::Int64;
	return VariableValue;
}

FFormulaValue FFormulaValue::Create(const uint8 Value)
{
	FFormulaValue VariableValue = FFormulaValue();
	VariableValue.Value.Set<uint32>(Value);
	VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, UInt32Value));
	VariableValue.TypeCode = EFormulaValueType::UInt32;
	return VariableValue;
}

FFormulaValue FFormulaValue::Create(const uint16 Value)
{
	FFormulaValue VariableValue = FFormulaValue();
	VariableValue.Value.Set<uint32>(Value);
	VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, UInt32Value));
	VariableValue.TypeCode = EFormulaValueType::UInt32;
	return VariableValue;
}

FFormulaValue FFormulaValue::Create(const uint32 Value)
{
	FFormulaValue VariableValue = FFormulaValue();
	VariableValue.Value.Set<uint32>(Value);
	VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, UInt32Value));
	VariableValue.TypeCode = EFormulaValueType::UInt32;
	return VariableValue;
}

FFormulaValue FFormulaValue::Create(const uint64 Value)
{
	FFormulaValue VariableValue = FFormulaValue();
	VariableValue.Value.Set<uint64>(Value);
	VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, UInt64Value));
	VariableValue.TypeCode = EFormulaValueType::UInt64;
	return VariableValue;
}

FFormulaValue FFormulaValue::Create(const float Value)
{
	FFormulaValue VariableValue = FFormulaValue();
	VariableValue.Value.Set<float>(Value);
	VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, FloatValue));
	VariableValue.TypeCode = EFormulaValueType::Float;
	return VariableValue;
}

FFormulaValue FFormulaValue::Create(const double Value)
{
	FFormulaValue VariableValue = FFormulaValue();
	VariableValue.Value.Set<double>(Value);
	VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, DoubleValue));
	VariableValue.TypeCode = EFormulaValueType::Double;
	return VariableValue;
}

FFormulaValue FFormulaValue::Create(const FTimespan Value)
{
	FFormulaValue VariableValue = FFormulaValue();
	VariableValue.Value.Set<FTimespan>(Value);
	VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, FTimespanValue));
	VariableValue.TypeCode = EFormulaValueType::Timespan;
	return VariableValue;
}

FFormulaValue FFormulaValue::Create(const FDateTime Value)
{
	FFormulaValue VariableValue = FFormulaValue();
	VariableValue.Value.Set<FDateTime>(Value);
	VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, FDateTimeValue));
	VariableValue.TypeCode = EFormulaValueType::DateTime;
	return VariableValue;
}

FFormulaValue FFormulaValue::Create(const FString& Value)
{
	FFormulaValue VariableValue = FFormulaValue();
	VariableValue.Value.Set<FString>(Value);
	VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, FStringValue));
	VariableValue.TypeCode = EFormulaValueType::String;
	return VariableValue;
}

FFormulaValue FFormulaValue::Create(const FName Value)
{
	FFormulaValue VariableValue = FFormulaValue();
	VariableValue.Value.Set<FName>(Value);
	VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, FNameValue));
	VariableValue.TypeCode = EFormulaValueType::Name;
	return VariableValue;
}

FFormulaValue FFormulaValue::Create(const FText& Value)
{
	FFormulaValue VariableValue = FFormulaValue();
	VariableValue.Value.Set<FText>(Value);
	VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, FTextValue));
	VariableValue.TypeCode = EFormulaValueType::Text;
	return VariableValue;
}

FFormulaValue FFormulaValue::Create(const TObjectPtr<UObject> Value)
{
	FFormulaValue VariableValue = FFormulaValue();
	VariableValue.Value.Set<UObject*>(Value.Get());
	VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, ObjectPtrValue));
	VariableValue.TypeCode = EFormulaValueType::ObjectPtr;
	return VariableValue;
}

FFormulaValue FFormulaValue::Create(UObject* Value)
{
	FFormulaValue VariableValue = FFormulaValue();
	if (Value)
	{
		VariableValue.Value.Set<UObject*>(Value);
		VariableValue.Type = GetStructProperty(GET_MEMBER_NAME_CHECKED(FFormulaVariableValueStruct, ObjectPtrValue));
		VariableValue.TypeCode = EFormulaValueType::ObjectPtr;
	}
	return VariableValue;
}

bool FFormulaValue::TryCopyCompleteValue(const FProperty* DestinationType, void* DestinationPtr) const
{
	EFormulaValueType ToType = GetPropertyTypeCode(DestinationType);
	
#if UE_VERSION_NEWER_THAN(5, 6, -1)
	if (const FOptionalProperty* OptionalProp = CastField<FOptionalProperty>(DestinationType))
	{
		if (this->IsNull())
		{
			OptionalProp->MarkUnset(DestinationPtr);
			return true;
		}
		else
		{
			if (OptionalProp->IsSet(DestinationPtr))
			{
				return TryCopyCompleteValue(OptionalProp->GetValueProperty(),
				                            OptionalProp->GetValuePointerForReplace(DestinationPtr));
			}
			else
			{
				bool bCopySuccess = TryCopyCompleteValue(OptionalProp->GetValueProperty(),
				                                         OptionalProp->MarkSetAndGetInitializedValuePointerToReplace(
					                                         DestinationPtr));
				if (!bCopySuccess)
				{
					OptionalProp->MarkUnset(DestinationPtr); // return back to unset state
				}
				return bCopySuccess;
			}
		}
	}
#endif

	// exact type copying
	if (this->Type->SameType(DestinationType))
	{
		CopyCompleteValue(DestinationPtr);
		return true;
	}

	// try UObject cast
	if (ToType == EFormulaValueType::ObjectPtr)
	{
		if (this->IsNull())
		{
			*static_cast<nullptr_t*>(DestinationPtr) = nullptr;
			return true;
		}
		else if (this->Value.IsType<UObject*>())
		{
			CopyCompleteValue(DestinationPtr);
			return true;
		}
	}
	
	// try widening number correction
	return Visit([ToType, DestinationPtr]<typename ValueType>(ValueType&& InValue) -> bool
	{
		using InT = std::decay_t<ValueType>;
		if constexpr (std::is_same_v<InT, bool>)
		{
			return false;
		}
		else if constexpr (std::is_integral_v<InT> || std::is_floating_point_v<InT>)
		{
			// Helper lambda to perform the actual assignment
			auto TryAssign = [InValue, DestinationPtr]<typename OutT>() -> bool
			{
				if constexpr (std::is_integral_v<InT> && std::is_integral_v<OutT>)
				{
					// Check if the value fits in the destination type without overflow
					if (!std::in_range<OutT, InT>(InValue))
					{
						return false;
					}
				}
				else if (std::is_floating_point_v<InT> &&
					(!std::is_floating_point_v<OutT> || sizeof(OutT) < sizeof(InT)))
				{
					// can't convert from floating point to non-floating point
					// and cannot narrow floating point
					return false;
				}
				*static_cast<OutT*>(DestinationPtr) = static_cast<const InT>(InValue);
				return true;
			};

			switch (ToType)
			{
				case EFormulaValueType::Int8:   return TryAssign.template operator()<int8>();
				case EFormulaValueType::Int16:  return TryAssign.template operator()<int16>();
				case EFormulaValueType::Int32:  return TryAssign.template operator()<int32>();
				case EFormulaValueType::Int64:  return TryAssign.template operator()<int64>();
				case EFormulaValueType::UInt8:  return TryAssign.template operator()<uint8>();
				case EFormulaValueType::UInt16: return TryAssign.template operator()<uint16>();
				case EFormulaValueType::UInt32: return TryAssign.template operator()<uint32>();
				case EFormulaValueType::UInt64: return TryAssign.template operator()<uint64>();
				case EFormulaValueType::Float:  return TryAssign.template operator()<float>();
				case EFormulaValueType::Double: return TryAssign.template operator()<double>();
				default: return false;
			}
		}
		else
		{
			return false; // can only promote numeric types
		}
	}, Value);
	
}

void FFormulaValue::CopyCompleteValue(void* DestinationPtr) const
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

FString FFormulaValue::ToString() const
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
	else if (this->Value.IsType<FDateTime>())
	{
		return this->Value.Get<FDateTime>().ToString();
	}
	else if (this->Value.IsType<FTimespan>())
	{
		return this->Value.Get<FTimespan>().ToString();
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
		return FString::Printf(TEXT("Struct Type: %s"), *this->GetType()->GetCPPType());
	}
}
