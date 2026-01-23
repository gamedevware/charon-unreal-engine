#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "Templates/SharedPointer.h"
#include "Misc/Timespan.h"
#include "Misc/DateTime.h"
#include "Internationalization/Text.h"
#include "UObject/NameTypes.h"
#include "UObject/Object.h"
#include "EFormulaValueType.h"
#include "TFormulaTypeMap.h"
#include "Misc/EngineVersionComparison.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFormulaValue, Log, All);

class CHARON_API FFormulaValue : public TSharedFromThis<FFormulaValue>
{
private:
	TArray<uint8> StructBytes;
	FProperty* Type;
	EFormulaValueType TypeCode;
public:
	static TSharedRef<FFormulaValue> Null();
	static TSharedRef<FFormulaValue> TrueBool();
	static TSharedRef<FFormulaValue> FalseBool();

	EFormulaValueType GetTypeCode() const { return this->TypeCode; }
	const FProperty* GetType() const { return this->Type; }

	bool IsNull() const { return  this->TypeCode == EFormulaValueType::Null; }

	FFormulaValue();
	FFormulaValue(FProperty* FieldType, const void* ValuePtr);
	template<typename T>
	FFormulaValue(T Value)
	{
		using FMapper = TFormulaTypeMap<T>;
		this->Type = FMapper::Type::GetLiteralProperty();
		this->TypeCode = FMapper::Type::TypeCode;
#if UE_VERSION_NEWER_THAN(5, 5, -1)
		this->StructBytes.SetNumZeroed(this->Type->GetElementSize());
#else
		this->StructBytes.SetNumZeroed(this->Type->ElementSize);
#endif
		this->Type->CopyCompleteValue(this->StructBytes.GetData(), &Value);
		this->SetNullIfObjectPtr();
	}
	FFormulaValue(const TObjectPtr<UObject> Value) : FFormulaValue(Value.Get()) { }
	// ReSharper restore CppNonExplicitConvertingConstructor

	/*
	 * Try to copy value with only data size and type size check (expands ints, floats if needed)
	 * doesn't (!) validate UClass references or UEnum values
	 */
	bool TryCopyCompleteValue(const FProperty* DestinationType, void* DestinationPtr) const;

	bool TrySetPropertyValue_InContainer(const FProperty* DestinationType, void* ContainerPtr, int32 ArrayIndex = 0) const;
	
	/*
	 * Visit each well known value type in this variant.
	 *
	 * Example:
	 * Value.VisitValue([]<typename ValueType>(const FProperty* Property, const ValueType&& InValue) -> void {  ...  }
	 * 
	*/
	template <typename VisitorFunc>
	decltype(auto) VisitValue(VisitorFunc&& Visitor) const
	{
		const void* DataPtr = this->StructBytes.GetData();
		EFormulaValueType FromTypeCode = this->TypeCode;

		// unwrap types to primitive value
		if (const FEnumProperty* EnumProperty = CastField<FEnumProperty>(this->Type))
		{
			FromTypeCode = GetPropertyTypeCode(EnumProperty->GetUnderlyingProperty());
		}
		
		switch (FromTypeCode) {
		case EFormulaValueType::Null: return Invoke(Forward<VisitorFunc>(Visitor), this->Type, *static_cast<const nullptr_t*>(DataPtr));
		case EFormulaValueType::Boolean: return Invoke(Forward<VisitorFunc>(Visitor), this->Type, *static_cast<const bool*>(DataPtr));
		case EFormulaValueType::UInt8: return Invoke(Forward<VisitorFunc>(Visitor), this->Type, *static_cast<const uint8*>(DataPtr));
		case EFormulaValueType::UInt16: return Invoke(Forward<VisitorFunc>(Visitor), this->Type, *static_cast<const uint16*>(DataPtr));
		case EFormulaValueType::UInt32: return Invoke(Forward<VisitorFunc>(Visitor), this->Type, *static_cast<const uint32*>(DataPtr));
		case EFormulaValueType::UInt64: return Invoke(Forward<VisitorFunc>(Visitor), this->Type, *static_cast<const uint64*>(DataPtr));
		case EFormulaValueType::Int8: return Invoke(Forward<VisitorFunc>(Visitor), this->Type, *static_cast<const int8*>(DataPtr));
		case EFormulaValueType::Int16: return Invoke(Forward<VisitorFunc>(Visitor), this->Type, *static_cast<const int16*>(DataPtr));
		case EFormulaValueType::Int32: return Invoke(Forward<VisitorFunc>(Visitor), this->Type, *static_cast<const int32*>(DataPtr));
		case EFormulaValueType::Int64: return Invoke(Forward<VisitorFunc>(Visitor), this->Type, *static_cast<const int64*>(DataPtr));
		case EFormulaValueType::Float: return Invoke(Forward<VisitorFunc>(Visitor), this->Type, *static_cast<const float*>(DataPtr));
		case EFormulaValueType::Double: return Invoke(Forward<VisitorFunc>(Visitor), this->Type, *static_cast<const double*>(DataPtr));
		case EFormulaValueType::Timespan: return Invoke(Forward<VisitorFunc>(Visitor), this->Type, *static_cast<const FTimespan*>(DataPtr));
		case EFormulaValueType::DateTime: return Invoke(Forward<VisitorFunc>(Visitor), this->Type, *static_cast<const FDateTime*>(DataPtr));
		case EFormulaValueType::String: return Invoke(Forward<VisitorFunc>(Visitor), this->Type, *static_cast<const FString*>(DataPtr));
		case EFormulaValueType::Name: return Invoke(Forward<VisitorFunc>(Visitor), this->Type, *static_cast<const FName*>(DataPtr));
		case EFormulaValueType::Text: return Invoke(Forward<VisitorFunc>(Visitor), this->Type, *static_cast<const FText*>(DataPtr));
		case EFormulaValueType::ObjectPtr: return Invoke(Forward<VisitorFunc>(Visitor), this->Type, *static_cast<UObject* const*>(DataPtr));
		default: return Invoke(Forward<VisitorFunc>(Visitor), this->Type, *this);
		}
	}
	
	bool TryGetBoolean(bool& OutValue) const;
	bool TryGetInt32(int32& OutValue) const;
	bool TryGetInt64(int64& OutValue) const;
	bool TryGetDouble(double& OutValue) const;
	bool TryGetContainerAddress(void*& OutValue);

	FString ToString() const;

private:
	void SetNullIfObjectPtr();
};
