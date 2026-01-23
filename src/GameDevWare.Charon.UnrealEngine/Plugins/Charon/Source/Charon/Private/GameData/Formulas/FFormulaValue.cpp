#include "GameData/Formulas/FFormulaValue.h"
#include "utility"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Misc/EngineVersionComparison.h"
#include "UObject/EnumProperty.h"
#include "UObject/FieldPathProperty.h"
#include "UObject/TextProperty.h"
#include "UObject/UnrealType.h"
#if __has_include("UObject/StrProperty.h")
#include "UObject/StrProperty.h"
#endif

DEFINE_LOG_CATEGORY(LogFormulaValue);

static TSharedPtr<FFormulaValue> NullValue;  
TSharedRef<FFormulaValue> FFormulaValue::Null()
{
	if (!NullValue.IsValid())
	{
		NullValue = MakeShared<FFormulaValue>(nullptr);
	}
	return NullValue.ToSharedRef();
}
static TSharedPtr<FFormulaValue> TrueBoolValue;  
TSharedRef<FFormulaValue> FFormulaValue::TrueBool()
{
	if (!TrueBoolValue.IsValid())
	{
		TrueBoolValue = MakeShared<FFormulaValue>(true);
	}
	return TrueBoolValue.ToSharedRef();
}
static TSharedPtr<FFormulaValue> FalseBoolValue;  
TSharedRef<FFormulaValue> FFormulaValue::FalseBool()
{
	if (!FalseBoolValue.IsValid())
	{
		FalseBoolValue = MakeShared<FFormulaValue>(false);
	}
	return FalseBoolValue.ToSharedRef();
}

FFormulaValue::FFormulaValue()
{
	this->StructBytes.SetNumZeroed(sizeof(nullptr_t));
	this->Type = FDotNetObject::GetNullLiteralProperty();
	this->TypeCode = EFormulaValueType::Null;
}

FFormulaValue::FFormulaValue(FProperty* FieldType, const void* ValuePtr)
{
	check(FieldType);
	check(ValuePtr);
	
	this->Type = FieldType;

#if UE_VERSION_NEWER_THAN(5, 5, -1)
	const int32 ElementSize = FieldType->GetElementSize();
#else
	const int32 ElementSize = FieldType->ElementSize;
#endif
	
	
	this->StructBytes.SetNumZeroed(ElementSize);
	this->Type->CopyCompleteValue(this->StructBytes.GetData(), ValuePtr);
	this->TypeCode = GetPropertyTypeCode(this->Type);

	this->SetNullIfObjectPtr();
}

bool FFormulaValue::TryCopyCompleteValue(const FProperty* DestinationType, void* DestinationPtr) const
{
	check(DestinationType);
	check(DestinationPtr);
	
	EFormulaValueType ToTypeCode = GetPropertyTypeCode(DestinationType);

	// unwrap types to primitive values
	if (const FEnumProperty* EnumProperty = CastField<FEnumProperty>(DestinationType))
	{
		ToTypeCode = GetPropertyTypeCode(EnumProperty->GetUnderlyingProperty());
	}
	
	// exact type copying
	if (this->Type->SameType(DestinationType))
	{
		this->Type->CopyCompleteValue(DestinationPtr, this->StructBytes.GetData());
		return true;
	}

	// try UObject pointer
	if (ToTypeCode == EFormulaValueType::ObjectPtr)
	{
		if (this->IsNull())
		{
			*static_cast<nullptr_t*>(DestinationPtr) = nullptr;
			return true;
		}
		else if (this->TypeCode == EFormulaValueType::ObjectPtr)
		{
			this->Type->CopyCompleteValue(DestinationPtr, this->StructBytes.GetData());
			return true;
		}
	}
	
	// try coerce numeric value
	return this->VisitValue([ToTypeCode, DestinationPtr]<typename ValueType>(FProperty* _, const ValueType& InValue) -> bool
	{
		using InT = std::decay_t<ValueType>;
		constexpr bool bIsInteger = std::is_integral_v<InT>;
		constexpr bool bIsFloat = std::is_floating_point_v<InT>;
		constexpr bool bIsBool = std::is_same_v<InT, bool>;
		
		if constexpr ((bIsInteger || bIsFloat) && !bIsBool)
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

			switch (ToTypeCode)
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
				default: break;
			}
		}
		
		return false; // only types are promoted
	});
}

bool FFormulaValue::TrySetPropertyValue_InContainer(const FProperty* DestinationType, void* ContainerPtr, int32 ArrayIndex) const
{
	return this->VisitValue([DestinationType, ContainerPtr, ArrayIndex]<typename ValueType>(const FProperty* ThisType, const ValueType& InValue) -> bool
	{
		using InT = std::decay_t<ValueType>;
		
		if (const FBoolProperty* BoolProperty = CastField<FBoolProperty>(DestinationType))
		{
			if constexpr (std::is_same_v<InT, bool>)
			{
				BoolProperty->SetPropertyValue_InContainer(ContainerPtr, InValue, ArrayIndex);
			}
		}
		else if (const FNumericProperty* NumericProp = CastField<FNumericProperty>(DestinationType))
		{
			if constexpr (std::is_floating_point_v<InT> || std::is_integral_v<InT>)
			{
				if (NumericProp->IsFloatingPoint())
				{
					NumericProp->SetFloatingPointPropertyValue(NumericProp->ContainerPtrToValuePtr<void>(ContainerPtr, ArrayIndex), InValue);
				}
				else if (std::is_signed_v<InT>)
				{
					NumericProp->SetIntPropertyValue(NumericProp->ContainerPtrToValuePtr<void>(ContainerPtr, ArrayIndex), static_cast<int64>(InValue));
				}
				else
				{
					NumericProp->SetIntPropertyValue(NumericProp->ContainerPtrToValuePtr<void>(ContainerPtr, ArrayIndex), static_cast<uint64>(InValue));
				}
			}
		}
		else if (const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(DestinationType))
		{
			if constexpr (std::is_same_v<InT, UObject*>)
			{
				ObjectProperty->SetObjectPropertyValue_InContainer(ContainerPtr, InValue, ArrayIndex);
			}
		}
		else if (const FStructProperty* StructProp = CastField<FStructProperty>(DestinationType))
		{
			if (StructProp->SameType(ThisType))
			{
				StructProp->SetSingleValue_InContainer(ContainerPtr, &InValue, ArrayIndex);
			}
		}
		else if (const FEnumProperty* EnumProp = CastField<FEnumProperty>(DestinationType))
		{
			const FNumericProperty* EnumNumProperty = EnumProp->GetUnderlyingProperty();
			if constexpr (std::is_floating_point_v<InT> || std::is_integral_v<InT>)
			{
				if (EnumNumProperty->IsFloatingPoint())
				{
					EnumNumProperty->SetFloatingPointPropertyValue(EnumNumProperty->ContainerPtrToValuePtr<void>(ContainerPtr, ArrayIndex), InValue);
				}
				else if (std::is_signed_v<InT>)
				{
					EnumNumProperty->SetIntPropertyValue(EnumNumProperty->ContainerPtrToValuePtr<void>(ContainerPtr, ArrayIndex), static_cast<int64>(InValue));
				}
				else
				{
					EnumNumProperty->SetIntPropertyValue(EnumNumProperty->ContainerPtrToValuePtr<void>(ContainerPtr, ArrayIndex), static_cast<uint64>(InValue));
				}
			}
		}
		else if (const FStrProperty* StrProp = CastField<FStrProperty>(DestinationType))
		{
			if constexpr (std::is_same_v<InT, FString>)
			{
				StrProp->SetPropertyValue_InContainer(ContainerPtr, InValue, ArrayIndex);
			}
			else if constexpr (std::is_same_v<InT, FText> || std::is_same_v<InT, FName>)
			{
				StrProp->SetPropertyValue_InContainer(ContainerPtr, InValue.ToString(), ArrayIndex);
			}
		}
		else if (const FTextProperty* TextProp = CastField<FTextProperty>(DestinationType))
		{
			if constexpr (std::is_same_v<InT, FString>)
			{
				TextProp->SetPropertyValue_InContainer(ContainerPtr, FText::FromString(InValue), ArrayIndex);
			}
			else if constexpr (std::is_same_v<InT, FText>)
			{
				TextProp->SetPropertyValue_InContainer(ContainerPtr, InValue, ArrayIndex);
			}
			if constexpr (std::is_same_v<InT, FName>)
			{
				TextProp->SetPropertyValue_InContainer(ContainerPtr, FText::FromName(InValue), ArrayIndex);
			}
		}
		else if (const FNameProperty* NameProp = CastField<FNameProperty>(DestinationType))
		{
			if constexpr (std::is_same_v<InT, FString>)
			{
				NameProp->SetPropertyValue_InContainer(ContainerPtr, FName(InValue), ArrayIndex);
			}
			else if constexpr (std::is_same_v<InT, FText>)
			{
				NameProp->SetPropertyValue_InContainer(ContainerPtr, FName(InValue.ToString()), ArrayIndex);
			}
			else if constexpr (std::is_same_v<InT, FName>)
			{
				NameProp->SetPropertyValue_InContainer(ContainerPtr, InValue, ArrayIndex);
			}
		}
		return false;
	});
}

bool FFormulaValue::TryGetBoolean(bool& OutValue) const
{
	if (const FBoolProperty* BoolProperty = CastField<FBoolProperty>(this->Type))
	{
		OutValue = BoolProperty->GetPropertyValue(this->StructBytes.GetData());
		return true;
	}
	OutValue = false;
	return false;
}

bool FFormulaValue::TryGetInt32(int32& OutValue) const
{
	return this->TryCopyCompleteValue(FDotNetInt32::GetLiteralProperty(), &OutValue);
}

bool FFormulaValue::TryGetInt64(int64& OutValue) const
{
	return this->TryCopyCompleteValue(FDotNetInt64::GetLiteralProperty(), &OutValue);
}

bool FFormulaValue::TryGetDouble(double& OutValue) const
{
	return this->TryCopyCompleteValue(FDotNetDouble::GetLiteralProperty(), &OutValue);
}

bool FFormulaValue::TryGetContainerAddress(void*& OutValue)
{
	if (const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(this->Type))
	{
		OutValue = ObjectProperty->GetObjectPropertyValue(this->StructBytes.GetData());
		return true;
	}
	else if (CastField<FStructProperty>(this->Type) ||
		CastField<FArrayProperty>(this->Type) ||
		CastField<FMapProperty>(this->Type) ||
		CastField<FSetProperty>(this->Type))
	{
		OutValue = this->StructBytes.GetData();
		return  true;
	}
	
	OutValue = nullptr;
	return false;
}

void FFormulaValue::SetNullIfObjectPtr()
{
	if (const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(this->Type))
	{
		const UObject* ObjectPtr = ObjectProperty->GetObjectPropertyValue(this->StructBytes.GetData());
		if (!ObjectPtr)
		{
			this->Type = FDotNetObject::GetNullLiteralProperty();
			this->TypeCode = EFormulaValueType::Null;
		}
	}
}

FString FFormulaValue::ToString() const
{
	if (this->TypeCode == EFormulaValueType::Null)
	{
		return TEXT("null");
	}

	FString StructText;
	const void* DataPtr = this->StructBytes.GetData();
	this->Type->ExportText_Direct(StructText, DataPtr, DataPtr, nullptr, 0);
	return StructText;
}
