// Copyright GameDevWare, Denis Zykov 2025

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

/**
 * A specialized variant type used for formula interpretation.
 * * This class acts as a container for various Unreal Engine types (primitives, structs, objects)
 * during expression evaluation. It handles internal memory management, tracks type metadata
 * via FProperty, and manages the lifetime of complex structures.
 */
class CHARON_API FFormulaValue : public TSharedFromThis<FFormulaValue>
{
private:
	TArray<uint8> StructBytes;
	FProperty& Type;
	EFormulaValueType const TypeCode;

	template<typename T>
	static bool IsNullValue(T Value)
	{
		if constexpr (std::is_same_v<T, UObject*>)
			return Value == nullptr;
		else if constexpr (std::is_same_v<T, nullptr_t>)
			return true;
		else
			return false;
	}
	static bool IsNullValue(FProperty* ValueType, const void* ValuePtr)
	{
		check(ValueType);
		check(ValuePtr);
		
		const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(ValueType);
		return ObjectProperty && ObjectProperty->GetObjectPropertyValue(ValuePtr) == nullptr;
	}
	
	FFormulaValue();
public:
	/** Creates a formula value representing a null/undefined state. */
	static TSharedRef<FFormulaValue> Null();
	/** Creates a formula value representing a boolean 'true'. */
	static TSharedRef<FFormulaValue> TrueBool();
	/** Creates a formula value representing a boolean 'false'. */
	static TSharedRef<FFormulaValue> FalseBool();

	/** * Gets the high-level type category of the stored value.
	 * @return An EFormulaValueType enum representing the underlying data type.
	 */
	EFormulaValueType GetTypeCode() const { return this->TypeCode; }

	/** * Gets the Unreal Engine FProperty associated with this value.
	 * @note Metadata like property name or offset is arbitrary; only type information is valid.
	 * @return The FProperty pointer used for reflection-based operations.
	 */
	FProperty* GetType() const { return &Type; } 

	/** * Gets the Unreal Engine FProperty associated with this value.
	 * @note Metadata like property name or offset is arbitrary; only type information is valid.
	 * @return The FProperty pointer used for reflection-based operations.
	 */
	FString GetCPPType() const;
	
	/** * Checks if the value is null. 
	 * Note that unset Optionals or null Object pointers are normalized to EFormulaValueType::Null.
	 */
	bool IsNull() const { return  this->TypeCode == EFormulaValueType::Null; }

	FFormulaValue(FProperty* ValueType);
	explicit FFormulaValue(FProperty* ValueType, const void* ValuePtr);
	
	template<typename T>
	FFormulaValue(T Value):
		Type(IsNullValue<T>(Value) ? *UDotNetObject::GetNullLiteralProperty() : *TFormulaTypeMap<T>::Type::GetLiteralProperty()),
		TypeCode(IsNullValue<T>(Value) ? EFormulaValueType::Null : TFormulaTypeMap<T>::Type::TypeCode)
	{
		static_assert(TIsFormulaTypeMapped<T>::value, "Type is not mapped in TFormulaTypeMap. Please add a specialization: template<> struct TFormulaTypeMap<YourType> { using Type = ...; };");
		
		checkSlow(this->Type.GetSize() > 0);
		
		this->StructBytes.SetNumZeroed(this->Type.GetSize());
		this->Type.CopyCompleteValue(this->StructBytes.GetData(), &Value);
	}
	FFormulaValue(const TObjectPtr<UObject> Value) : FFormulaValue(Value.Get()) { }
	// ReSharper restore CppNonExplicitConvertingConstructor
	~FFormulaValue();

	/**
	 * Validates and copies the internal value to a destination memory address.
	 * * @param DestinationType The target FProperty type to copy to.
	 * @param DestinationPtr Memory address where the value should be written.
	 * @return True if the copy succeeded, false if types are incompatible.
	 */
	bool TryCopyCompleteValue(const FProperty* DestinationType, void* DestinationPtr) const;
	
	/**
	 * Validates and sets the value within a target container (Object or Struct).
	 * * @param DestinationType The property type within the container.
	 * @param ContainerPtr Pointer to the start of the struct or object instance.
	 * @param ArrayIndex The index to set if the target is an array property.
	 * @return True if the property was successfully set.
	 */
	bool TrySetPropertyValue_InContainer(const FProperty* DestinationType, void* ContainerPtr, int32 ArrayIndex = 0) const;
	
	/**
	 * Executes a lambda or functor based on the underlying stored type.
	 * * The visitor should accept: `(FProperty* Property, const ValueType& Value)`.
	 * Enums are automatically unwrapped to their underlying primitive types before visiting.
	 * * @param Visitor The callable to execute.
	 * @return The result of the visitor function.
	 */
	template <typename VisitorFunc>
	decltype(auto) VisitValue(VisitorFunc&& Visitor) const
	{
		const void* DataPtr = this->GetStructPtrChecked();
		
		EFormulaValueType FromTypeCode = this->TypeCode;

		// unwrap types to primitive value
		if (const FEnumProperty* EnumProperty = CastField<FEnumProperty>(&this->Type))
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
		case EFormulaValueType::Struct: return Invoke(Forward<VisitorFunc>(Visitor), this->Type, *static_cast<UStruct*>(const_cast<void*>(DataPtr)));
		default: return Invoke(Forward<VisitorFunc>(Visitor), this->Type, *this);
		}
	}

	/** Attempts to extract the value as a boolean. Returns false if conversion is impossible. */
	bool TryGetBoolean(bool& OutValue) const;
	/** Attempts to extract the value as an int32. Returns false if conversion is impossible. */
	bool TryGetInt32(int32& OutValue) const;
	/** Attempts to extract the value as an int64. Returns false if conversion is impossible. */
	bool TryGetInt64(int64& OutValue) const;
	/** Attempts to extract the value as a UObject*. Returns false if conversion is impossible. */
	bool TryGetObjectPtr(UObject*& OutValue) const;
	
	/**
	 * Retrieves the raw memory address for complex types.
	 * Useful for accessing UObjects, Arrays, Sets, Maps, or Structs directly.
	 * * @param OutValue Output reference to the address pointer.
	 * @return True if the value is a container/complex type and address was retrieved.
	 */
	bool TryGetContainerAddress(void*& OutValue);

	/** Returns a string representation of the stored value for debugging or logging. */
	FString ToString() const;

	/** Returns a string representation of the full C++ name of the property's value. */
	static FString GetExtendedCppName(const FProperty* Property);
private:
	bool TryCopyObjectValue(const FObjectPropertyBase* DestinationObjectProperty, void* DestinationPtr) const;
	bool TryCopyNumericValue(const FProperty* DestinationType, void* DestinationPtr) const;
	bool TryCopyStringValue(const FStrProperty* DestinationStringProperty, void* DestinationPtr) const;
	bool TryCopyTextValue(const FTextProperty* DestinationTextProperty, void* DestinationPtr) const;
	bool TryCopyNameProperty(const FNameProperty* DestinationNameProperty, void* DestinationPtr) const;

	void* GetStructPtrChecked();
	const void* GetStructPtrChecked() const;
};
