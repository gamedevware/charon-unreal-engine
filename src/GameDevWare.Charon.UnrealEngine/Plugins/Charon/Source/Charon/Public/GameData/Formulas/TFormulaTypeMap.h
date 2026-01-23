#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "EFormulaValueType.h"
#include "DotNetTypes/FDotNetBoolean.h"
#include "DotNetTypes/FDotNetDateTime.h"
#include "DotNetTypes/FDotNetDouble.h"
#include "DotNetTypes/FDotNetInt16.h"
#include "DotNetTypes/FDotNetInt32.h"
#include "DotNetTypes/FDotNetInt64.h"
#include "DotNetTypes/FDotNetInt8.h"
#include "DotNetTypes/FDotNetName.h"
#include "DotNetTypes/FDotNetObject.h"
#include "DotNetTypes/UDotNetSingle.h"
#include "DotNetTypes/FDotNetString.h"
#include "DotNetTypes/FDotNetText.h"
#include "DotNetTypes/FDotNetTimeSpan.h"
#include "DotNetTypes/FDotNetUInt16.h"
#include "DotNetTypes/FDotNetUInt32.h"
#include "DotNetTypes/FDotNetUInt64.h"
#include "DotNetTypes/FDotNetUInt8.h"
#include "utility"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Internationalization/Text.h"
#include "Misc/DateTime.h"
#include "Misc/EngineVersionComparison.h"
#include "Misc/Timespan.h"
#include "UObject/EnumProperty.h"
#include "UObject/FieldPathProperty.h"
#include "UObject/NameTypes.h"
#include "UObject/Object.h"
#include "UObject/TextProperty.h"
#include "UObject/UnrealType.h"
#if __has_include("UObject/StrProperty.h")
#include "UObject/StrProperty.h"
#endif

template<typename T> struct TFormulaTypeMap;
template<> struct TFormulaTypeMap<bool>			{ using Type = FDotNetBoolean; };
template<> struct TFormulaTypeMap<int8>			{ using Type = FDotNetInt8; };
template<> struct TFormulaTypeMap<int16>		{ using Type = FDotNetInt16; };
template<> struct TFormulaTypeMap<int32>		{ using Type = FDotNetInt32; };
template<> struct TFormulaTypeMap<int64>		{ using Type = FDotNetInt64; };
template<> struct TFormulaTypeMap<uint8>		{ using Type = FDotNetUInt8; };
template<> struct TFormulaTypeMap<uint16>		{ using Type = FDotNetUInt16; };
template<> struct TFormulaTypeMap<uint32>		{ using Type = FDotNetUInt32; };
template<> struct TFormulaTypeMap<uint64>		{ using Type = FDotNetUInt64; };
template<> struct TFormulaTypeMap<float>		{ using Type = UDotNetSingle; };
template<> struct TFormulaTypeMap<double>		{ using Type = FDotNetDouble; };
template<> struct TFormulaTypeMap<FString>		{ using Type = FDotNetString; };
template<> struct TFormulaTypeMap<FName>		{ using Type = FDotNetName; };
template<> struct TFormulaTypeMap<FText>		{ using Type = FDotNetText; };
template<> struct TFormulaTypeMap<FTimespan>	{ using Type = FDotNetTimeSpan; };
template<> struct TFormulaTypeMap<FDateTime>	{ using Type = FDotNetDateTime; };
template<> struct TFormulaTypeMap<UObject*>		{ using Type = FDotNetObject; };
template<> struct TFormulaTypeMap<nullptr_t>	{ using Type = FDotNetObject; };

inline EFormulaValueType GetPropertyTypeCode(const FProperty* FieldType)
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
		if (FieldType == FDotNetObject::GetNullLiteralProperty())
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
		const auto TimespanStruct =  FDotNetTimeSpan::GetLiteralProperty()->Struct;
		const auto DateTimeStruct = FDotNetDateTime::GetLiteralProperty()->Struct;

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
	else
	{
		return EFormulaValueType::Struct;
	}
}