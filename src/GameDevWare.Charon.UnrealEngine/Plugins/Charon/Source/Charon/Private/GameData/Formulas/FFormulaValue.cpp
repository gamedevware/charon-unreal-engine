// Copyright GameDevWare, Denis Zykov 2025

#include "GameData/Formulas/FFormulaValue.h"
#include "utility"
#include <limits>
#include <type_traits>
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "UObject/EnumProperty.h"
#include "UObject/FieldPathProperty.h"
#include "UObject/TextProperty.h"
#include "UObject/UnrealType.h"
#if __has_include("UObject/StrProperty.h")
#include "UObject/StrProperty.h"
#endif

#include "Misc/EngineVersionComparison.h"

DEFINE_LOG_CATEGORY(LogFormulaValue);

TSharedRef<FFormulaValue> FFormulaValue::Null()
{
	static TSharedPtr<FFormulaValue> NullValue = MakeShared<FFormulaValue>(nullptr);
	return NullValue.ToSharedRef();
}
TSharedRef<FFormulaValue> FFormulaValue::TrueBool()
{
	static TSharedPtr<FFormulaValue> TrueBoolValue = MakeShared<FFormulaValue>(true);
	return TrueBoolValue.ToSharedRef();
}
TSharedRef<FFormulaValue> FFormulaValue::FalseBool()
{
	static TSharedPtr<FFormulaValue> FalseBoolValue = MakeShared<FFormulaValue>(false);
	return FalseBoolValue.ToSharedRef();
}

FString FFormulaValue::GetCPPType() const
{
	if (this->IsNull())
	{
		return TEXT("nullptr_t");
	}
	return GetExtendedCppName(this->GetType());
	
}

FFormulaValue::FFormulaValue():
	Type(*UDotNetObject::GetNullLiteralProperty()),
	TypeCode(EFormulaValueType::Null)
{
	checkSlow(this->Type.GetSize() > 0);
	
	this->StructBytes.SetNumZeroed(this->Type.GetSize());
}

FFormulaValue::FFormulaValue(FProperty* ValueType, const void* ValuePtr):
	Type(IsNullValue(ValueType, ValuePtr) ? *UDotNetObject::GetNullLiteralProperty() : *ValueType),
	TypeCode(IsNullValue(ValueType, ValuePtr) ? EFormulaValueType::Null : GetPropertyTypeCode(&this->Type))
{
	check(ValueType);
	check(ValuePtr);
	checkSlow(this->Type.GetSize() > 0);
	
	this->StructBytes.SetNumZeroed(this->Type.GetSize());
	this->Type.CopyCompleteValue(GetStructPtrChecked(), ValuePtr);
}

FFormulaValue::~FFormulaValue()
{
	// UStruct/FString/FArrays etc stored in StructBytes are always in valid state and properly initialized
	// and obtained with FProperty:CopyCompleteValue, so we need to 'release' them using  DestroyValue method

	void* StructPtr = this->StructBytes.GetData();
	if (StructPtr && this->StructBytes.Num() > 0)
	{
		this->Type.DestroyValue(StructPtr);
	}
}

bool FFormulaValue::TryCopyCompleteValue(const FProperty* DestinationType, void* DestinationPtr) const
{
	check(DestinationType);
	check(DestinationPtr);

	if (this->Type.ArrayDim != DestinationType->ArrayDim)
	{
		return false; // type's arity is not same
	}
	
	// exact type copying
	if (this->Type.SameType(DestinationType))
	{
		this->Type.CopyCompleteValue(DestinationPtr, GetStructPtrChecked());
		return true;
	}

	// try coerce string
	if (const FStrProperty* DestinationStringProperty = CastField<FStrProperty>(DestinationType))
	{
		return TryCopyStringValue(DestinationStringProperty, DestinationPtr);
	}

	// try coerce text
	if (const FTextProperty* DestinationTextProperty = CastField<FTextProperty>(DestinationType))
	{
		return TryCopyTextValue(DestinationTextProperty, DestinationPtr);
	}

	// try coerce name
	if (const FNameProperty* DestinationNameProperty = CastField<FNameProperty>(DestinationType))
	{
		return TryCopyNameProperty(DestinationNameProperty, DestinationPtr);
	}

	// try to copy UObject pointer
	if (const FObjectPropertyBase* DestinationObjectProperty = CastField<FObjectPropertyBase>(DestinationType))
	{
		return TryCopyObjectValue(DestinationObjectProperty, DestinationPtr);
	}
	
	// try coerce numeric value
	return TryCopyNumericValue(DestinationType, DestinationPtr);
}

bool FFormulaValue::TrySetPropertyValue_InContainer(const FProperty* DestinationType, void* ContainerPtr, int32 ArrayIndex) const
{
	if (this->Type.ArrayDim != DestinationType->ArrayDim)
	{
		return false; // type's arity is not same
	}
	
	return this->VisitValue([DestinationType, ContainerPtr, ArrayIndex]<typename ValueType>(const FProperty& ThisType, const ValueType& InValue) -> bool
	{
		using InT = std::decay_t<ValueType>;
		
		if (const FBoolProperty* BoolProperty = CastField<FBoolProperty>(DestinationType))
		{
			if constexpr (std::is_same_v<InT, bool>)
			{
				BoolProperty->SetPropertyValue_InContainer(ContainerPtr, InValue, ArrayIndex);
				return true;
			}
		}
		else if (const FNumericProperty* NumericProp = CastField<FNumericProperty>(DestinationType))
		{
			if constexpr (std::is_floating_point_v<InT> || std::is_integral_v<InT>)
			{
				if (NumericProp->IsFloatingPoint())
				{
					NumericProp->SetFloatingPointPropertyValue(NumericProp->ContainerPtrToValuePtr<void>(ContainerPtr, ArrayIndex), InValue);
					return true;
				}
				else if (std::is_signed_v<InT>)
				{
					NumericProp->SetIntPropertyValue(NumericProp->ContainerPtrToValuePtr<void>(ContainerPtr, ArrayIndex), static_cast<int64>(InValue));
					return true;
				}
				else
				{
					NumericProp->SetIntPropertyValue(NumericProp->ContainerPtrToValuePtr<void>(ContainerPtr, ArrayIndex), static_cast<uint64>(InValue));
					return true;
				}
			}
		}
		else if (const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(DestinationType))
		{
			if constexpr (std::is_same_v<InT, UObject*>)
			{
				ObjectProperty->SetObjectPropertyValue_InContainer(ContainerPtr, InValue, ArrayIndex);
				return true;
			}
			else if constexpr (std::is_same_v<InT, nullptr_t>)
			{
				ObjectProperty->ClearValue_InContainer(ContainerPtr, ArrayIndex);
				return true;
			}
		}
		else if (const FStructProperty* StructProp = CastField<FStructProperty>(DestinationType))
		{
			if (StructProp->SameType(&ThisType))
			{
				StructProp->SetSingleValue_InContainer(ContainerPtr, &InValue, ArrayIndex);
				return true;
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
					return true;
				}
				else if (std::is_signed_v<InT>)
				{
					EnumNumProperty->SetIntPropertyValue(EnumNumProperty->ContainerPtrToValuePtr<void>(ContainerPtr, ArrayIndex), static_cast<int64>(InValue));
					return true;
				}
				else
				{
					EnumNumProperty->SetIntPropertyValue(EnumNumProperty->ContainerPtrToValuePtr<void>(ContainerPtr, ArrayIndex), static_cast<uint64>(InValue));
					return true;
				}
			}
		}
		else if (const FStrProperty* StrProp = CastField<FStrProperty>(DestinationType))
		{
			if constexpr (std::is_same_v<InT, FString>)
			{
				StrProp->SetPropertyValue_InContainer(ContainerPtr, InValue, ArrayIndex);
				return true;
			}
			else if constexpr (std::is_same_v<InT, FText> || std::is_same_v<InT, FName>)
			{
				StrProp->SetPropertyValue_InContainer(ContainerPtr, InValue.ToString(), ArrayIndex);
				return true;
			}
			else if constexpr (std::is_same_v<InT, nullptr_t>) // we allow nullify string-like values by clearing them
			{
				StrProp->ClearValue_InContainer(ContainerPtr, ArrayIndex);
				return true;
			}
		}
		else if (const FTextProperty* TextProp = CastField<FTextProperty>(DestinationType))
		{
			if constexpr (std::is_same_v<InT, FString>)
			{
				TextProp->SetPropertyValue_InContainer(ContainerPtr, FText::FromString(InValue), ArrayIndex);
				return true;
			}
			else if constexpr (std::is_same_v<InT, FText>)
			{
				TextProp->SetPropertyValue_InContainer(ContainerPtr, InValue, ArrayIndex);
				return true;
			}
			if constexpr (std::is_same_v<InT, FName>)
			{
				TextProp->SetPropertyValue_InContainer(ContainerPtr, FText::FromName(InValue), ArrayIndex);
				return true;
			}
			else if constexpr (std::is_same_v<InT, nullptr_t>) // we allow nullify string-like values by clearing them
			{
				TextProp->ClearValue_InContainer(ContainerPtr, ArrayIndex);
				return true;
			}
		}
		else if (const FNameProperty* NameProp = CastField<FNameProperty>(DestinationType))
		{
			if constexpr (std::is_same_v<InT, FString>)
			{
				NameProp->SetPropertyValue_InContainer(ContainerPtr, FName(InValue), ArrayIndex);
				return true;
			}
			else if constexpr (std::is_same_v<InT, FText>)
			{
				NameProp->SetPropertyValue_InContainer(ContainerPtr, FName(InValue.ToString()), ArrayIndex);
				return true;
			}
			else if constexpr (std::is_same_v<InT, FName>)
			{
				NameProp->SetPropertyValue_InContainer(ContainerPtr, InValue, ArrayIndex);
				return true;
			}
			else if constexpr (std::is_same_v<InT, nullptr_t>)
			{
				NameProp->ClearValue_InContainer(ContainerPtr, ArrayIndex); // we allow nullify string-like values by clearing them
				return true;
			}
		}
		return false;
	});
}

bool FFormulaValue::TryGetBoolean(bool& OutValue) const
{
	if (const FBoolProperty* BoolProperty = CastField<FBoolProperty>(&this->Type))
	{
		OutValue = BoolProperty->GetPropertyValue(GetStructPtrChecked());
		return true;
	}
	OutValue = false;
	return false;
}
bool FFormulaValue::TryGetInt32(int32& OutValue) const
{
	if (const FIntProperty* IntProperty = CastField<FIntProperty>(&this->Type))
	{
		OutValue = IntProperty->GetPropertyValue(GetStructPtrChecked());
		return true;
	}
	OutValue = 0;
	return false;
}
bool FFormulaValue::TryGetInt64(int64& OutValue) const
{
	if (const FInt64Property* IntProperty = CastField<FInt64Property>(&this->Type))
	{
		OutValue = IntProperty->GetPropertyValue(GetStructPtrChecked());
		return true;
	}
	OutValue = 0;
	return false;
}
bool FFormulaValue::TryGetDouble(double& OutValue) const
{
	if (const FDoubleProperty* DoubleProperty = CastField<FDoubleProperty>(&this->Type))
	{
		OutValue = DoubleProperty->GetPropertyValue(GetStructPtrChecked());
		return true;
	}
	OutValue = 0;
	return false;
}
bool FFormulaValue::TryGetObjectPtr(UObject*& OutValue) const
{
	if (const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(&this->Type))
	{
		OutValue = ObjectProperty->GetObjectPropertyValue(GetStructPtrChecked());
		return true;
	}
	OutValue = nullptr;
	return false;
}

bool FFormulaValue::TryGetContainerAddress(void*& OutValue)
{
	if (const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(&this->Type))
	{
		OutValue = ObjectProperty->GetObjectPropertyValue(GetStructPtrChecked());
		return true;
	}
	else if (CastField<FStructProperty>(&this->Type) ||
		CastField<FArrayProperty>(&this->Type) ||
		CastField<FMapProperty>(&this->Type) ||
		CastField<FSetProperty>(&this->Type))
	{
		OutValue = GetStructPtrChecked();
		return OutValue != nullptr;
	}
	
	OutValue = nullptr;
	return false;
}

FString FFormulaValue::ToString() const
{
	if (this->TypeCode == EFormulaValueType::Null)
	{
		return TEXT("null");
	}

	FString StructText;
	const void* DataPtr = GetStructPtrChecked();
	this->Type.ExportText_Direct(StructText, DataPtr, DataPtr, nullptr, 0);
	return StructText;
}

FString FFormulaValue::GetExtendedCppName(const FProperty* Property)
{
	check(Property);

	FString* ExtendedTypeText = nullptr;
	FString Name = Property->GetCPPType(ExtendedTypeText);
	return ExtendedTypeText ? Name + *ExtendedTypeText : Name;
}

bool FFormulaValue::TryCopyStringValue(const FStrProperty* DestinationStringProperty, void* DestinationPtr) const
{
	// FString is not nullable, but we allow it by clearing value
	if (this->IsNull())
	{
		DestinationStringProperty->ClearValue(DestinationPtr);
		return true;
	}
	else if (const FNameProperty* ThisNameProperty = CastField<FNameProperty>(&this->Type))
	{
		DestinationStringProperty->SetPropertyValue(
			DestinationPtr,
			ThisNameProperty->GetPropertyValue(GetStructPtrChecked()).ToString()
		);
		return true;
	}
	else if (const FTextProperty* ThisTextProperty = CastField<FTextProperty>(&this->Type))
	{
		DestinationStringProperty->SetPropertyValue(
			DestinationPtr,
			ThisTextProperty->GetPropertyValue(GetStructPtrChecked()).ToString()
		);
		return true;
	}
	else
	{
		return false; // not compatible
	}
}

bool FFormulaValue::TryCopyObjectValue(const FObjectPropertyBase* DestinationObjectProperty, void* DestinationPtr) const
{
	if (this->IsNull())
	{
		DestinationObjectProperty->SetObjectPropertyValue(DestinationPtr, nullptr);
		return true;
	}
	else if (const FObjectPropertyBase* ThisObjectProperty = CastField<FObjectPropertyBase>(&this->Type);
		ThisObjectProperty && ThisObjectProperty->PropertyClass->IsChildOf(DestinationObjectProperty->PropertyClass))
	{
		this->Type.CopyCompleteValue(DestinationPtr, GetStructPtrChecked());
		return true;
	}
	else
	{
		return false; // not compatible UObjects
	}
}

bool FFormulaValue::TryCopyNumericValue(const FProperty* DestinationType, void* DestinationPtr) const
{
	EFormulaValueType ToTypeCode = GetPropertyTypeCode(DestinationType);

	// unwrap types to primitive values
	if (const FEnumProperty* EnumProperty = CastField<FEnumProperty>(DestinationType))
	{
		ToTypeCode = GetPropertyTypeCode(EnumProperty->GetUnderlyingProperty());
	}
	
	return this->VisitValue([ToTypeCode, DestinationPtr]<typename ValueType>(FProperty&, const ValueType& InValue) -> bool
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
					if (!std::in_range<OutT>(InValue))
					{
						return false; // will lose some data on  conversion
					}
				}
				else if (std::is_floating_point_v<InT> && !std::is_floating_point_v<OutT>)
				{
					// can't convert from floating point to non-floating point
					// and cannot narrow floating point
					return false;
				}
				*static_cast<OutT*>(DestinationPtr) = static_cast<OutT>(InValue);
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

bool FFormulaValue::TryCopyTextValue(const FTextProperty* DestinationTextProperty, void* DestinationPtr) const
{
	// FText is not nullable, but we allow it by clearing value
	if (this->IsNull())
	{
		DestinationTextProperty->ClearValue(DestinationPtr);
		return true;
	}
	else if (const FNameProperty* ThisNameProperty = CastField<FNameProperty>(&this->Type))
	{
		DestinationTextProperty->SetPropertyValue(
			DestinationPtr,
			FText::FromString(ThisNameProperty->GetPropertyValue(GetStructPtrChecked()).ToString())
		);
		return true;
	}
	else if (const FStrProperty* ThisStringProperty = CastField<FStrProperty>(&this->Type))
	{
		DestinationTextProperty->SetPropertyValue(
			DestinationPtr,
			FText::FromString(ThisStringProperty->GetPropertyValue(GetStructPtrChecked()))
		);
		return true;
	}
	else
	{
		return false; // not compatible
	}
}

bool FFormulaValue::TryCopyNameProperty(const FNameProperty* DestinationNameProperty, void* DestinationPtr) const
{
	// FName is not nullable, but we allow it by clearing value
	if (this->IsNull())
	{
		DestinationNameProperty->ClearValue(DestinationPtr);
		return true;
	}
	else if (const FTextProperty* ThisTextProperty = CastField<FTextProperty>(&this->Type))
	{
		DestinationNameProperty->SetPropertyValue(
			DestinationPtr,
			FName(ThisTextProperty->GetPropertyValue(GetStructPtrChecked()).ToString())
		);
		return true;
	}
	else if (const FStrProperty* ThisStringProperty = CastField<FStrProperty>(&this->Type))
	{
		DestinationNameProperty->SetPropertyValue(
			DestinationPtr,
			FName(ThisStringProperty->GetPropertyValue(GetStructPtrChecked()))
		);
		return true;
	}
	else
	{
		return false; // not compatible
	}
}

void* FFormulaValue::GetStructPtrChecked()
{
	void* DataPtr = this->StructBytes.GetData();
	check(DataPtr);
	return DataPtr;
}
const void* FFormulaValue::GetStructPtrChecked() const
{
	const void* DataPtr = this->StructBytes.GetData();
	check(DataPtr);
	return DataPtr;
}