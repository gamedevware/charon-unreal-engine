// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "FFormulaFunction.h"
#include "FFormulaProperty.h"
#include "FFormulaValue.h"
#include "Containers/Array.h"
#include "Expressions/EBinaryOperationType.h"
#include "Expressions/EUnaryOperationType.h"
#include "Templates/SharedPointer.h"
#include "UObject/Class.h"

class CHARON_API IFormulaType : public TSharedFromThis<IFormulaType>
{
public:
	virtual ~IFormulaType() = default;
	
	virtual bool CanBeNull() const = 0;
	virtual bool IsAssignableFrom(UField* Type) const = 0;
	virtual bool IsAssignableFrom(const FProperty* ToType, bool bWithoutConversion);
	virtual FString GetCPPType() const = 0;
	virtual UField* GetTypeClassOrStruct() const = 0;
	virtual EFormulaValueType GetTypeCode() const = 0;
	virtual TSharedPtr<IFormulaType> GetUnderlyingType() const = 0;

	virtual const TArray<FString>& GetPropertyNames(bool bStatic) = 0;
	virtual const TArray<FString>& GetFunctionNames(bool bStatic) = 0;

	virtual bool TryGetBinaryOperation(EBinaryOperationType Operation, const FFormulaFunction*& FoundMOperation) = 0;
	virtual bool TryGetUnaryOperation(EUnaryOperationType Operation, const FFormulaFunction*& FoundMOperation) = 0;
	virtual bool TryGetConversionOperation(const FFormulaFunction*& FoundMOperation) = 0;

	virtual bool TryGetFunction(const FString& MemberName, bool bStatic, const FFormulaFunction*& FoundFunction) = 0;
	virtual bool TryGetProperty(const FString& MemberName, bool bStatic, const FFormulaProperty*& FoundProperty) = 0;
};

static TMap<EFormulaValueType, TSet<EFormulaValueType>> ConversionMaps = {
	{ EFormulaValueType::Null, TSet { EFormulaValueType::Null, EFormulaValueType::ObjectPtr } },
	{ EFormulaValueType::Boolean, TSet { EFormulaValueType::Boolean } },
	{ EFormulaValueType::UInt8, TSet { EFormulaValueType::UInt8 } },
	{ EFormulaValueType::UInt16, TSet { EFormulaValueType::UInt8, EFormulaValueType::UInt16 } },
	{ EFormulaValueType::UInt32, TSet { EFormulaValueType::UInt8, EFormulaValueType::UInt16, EFormulaValueType::UInt32 } },
	{ EFormulaValueType::UInt64, TSet { EFormulaValueType::UInt8, EFormulaValueType::UInt16, EFormulaValueType::UInt32, EFormulaValueType::UInt64 } },
	{ EFormulaValueType::Int8, TSet { EFormulaValueType::Int8 } },
	{ EFormulaValueType::Int16, TSet { EFormulaValueType::Int8, EFormulaValueType::UInt8, EFormulaValueType::Int16 } },
	{ EFormulaValueType::Int32, TSet { EFormulaValueType::Int8, EFormulaValueType::UInt8, EFormulaValueType::Int16, EFormulaValueType::UInt16, EFormulaValueType::Int32 } },
	{ EFormulaValueType::Int64, TSet { EFormulaValueType::Int8, EFormulaValueType::UInt8, EFormulaValueType::Int16, EFormulaValueType::UInt16, EFormulaValueType::Int32, EFormulaValueType::UInt32, EFormulaValueType::Int64 } },
	{ EFormulaValueType::Float, TSet { EFormulaValueType::Int8, EFormulaValueType::UInt8, EFormulaValueType::Int16, EFormulaValueType::UInt16, EFormulaValueType::Int32, EFormulaValueType::UInt32, EFormulaValueType::Int64, EFormulaValueType::UInt64, EFormulaValueType::Float } },
	{ EFormulaValueType::Double, TSet { EFormulaValueType::Int8, EFormulaValueType::UInt8, EFormulaValueType::Int16, EFormulaValueType::UInt16, EFormulaValueType::Int32, EFormulaValueType::UInt32, EFormulaValueType::Int64, EFormulaValueType::UInt64, EFormulaValueType::Float, EFormulaValueType::Double } },
	{ EFormulaValueType::Timespan, TSet { EFormulaValueType::Timespan } },
	{ EFormulaValueType::DateTime, TSet { EFormulaValueType::DateTime } },
	{ EFormulaValueType::String, TSet { EFormulaValueType::String, EFormulaValueType::Text, EFormulaValueType::Name, EFormulaValueType::Null } },
	{ EFormulaValueType::Name, TSet { EFormulaValueType::String, EFormulaValueType::Text, EFormulaValueType::Name, EFormulaValueType::Null } },
	{ EFormulaValueType::Text, TSet { EFormulaValueType::String, EFormulaValueType::Text, EFormulaValueType::Name, EFormulaValueType::Null } },
	{ EFormulaValueType::ObjectPtr, TSet { EFormulaValueType::Null } },
	{ EFormulaValueType::Struct, TSet<EFormulaValueType> { } },
	{ EFormulaValueType::Enum, TSet { EFormulaValueType::UInt8, EFormulaValueType::UInt16, EFormulaValueType::UInt32, EFormulaValueType::UInt64 } },
};
inline bool IFormulaType::IsAssignableFrom(const FProperty* ToType, const bool bWithoutConversion)
{
	const EFormulaValueType RequestedTypeCode = GetPropertyTypeCode(ToType);
	
	if (const FStructProperty* StructProp = CastField<FStructProperty>(ToType))
	{
		return IsAssignableFrom(StructProp->Struct.Get());
	}
	else if (const FObjectPropertyBase* ObjectProp = CastField<FObjectPropertyBase>(ToType))
	{
		return RequestedTypeCode == EFormulaValueType::Null || IsAssignableFrom(ObjectProp->PropertyClass.Get());
	}
	else if (bWithoutConversion)
	{
		const EFormulaValueType ThisTypeCode = this->GetTypeCode();
		const bool bIsObjectToNullCast = (ThisTypeCode == EFormulaValueType::ObjectPtr && RequestedTypeCode == EFormulaValueType::Null) ||
			(ThisTypeCode == EFormulaValueType::Null && RequestedTypeCode == EFormulaValueType::ObjectPtr);
		const bool bIsSameNonStructTypes = ThisTypeCode == RequestedTypeCode && ThisTypeCode != EFormulaValueType::Struct && ThisTypeCode != EFormulaValueType::ObjectPtr; 

		return bIsObjectToNullCast || bIsSameNonStructTypes; 
	}
	else
	{
		const EFormulaValueType ThisTypeCode = this->GetTypeCode();

		const TSet<EFormulaValueType>* CompatibleTypes = ConversionMaps.Find(ThisTypeCode);
		return CompatibleTypes && CompatibleTypes->Contains(RequestedTypeCode);
	}
}
