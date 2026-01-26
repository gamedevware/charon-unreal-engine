// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "EFormulaValueType.h"
#include "DotNetTypes/UDotNetBoolean.h"
#include "DotNetTypes/UDotNetDateTime.h"
#include "DotNetTypes/UDotNetDouble.h"
#include "DotNetTypes/UDotNetInt16.h"
#include "DotNetTypes/UDotNetInt32.h"
#include "DotNetTypes/UDotNetInt64.h"
#include "DotNetTypes/UDotNetInt8.h"
#include "DotNetTypes/UDotNetName.h"
#include "DotNetTypes/UDotNetObject.h"
#include "DotNetTypes/UDotNetSingle.h"
#include "DotNetTypes/UDotNetString.h"
#include "DotNetTypes/UDotNetText.h"
#include "DotNetTypes/UDotNetTimeSpan.h"
#include "DotNetTypes/UDotNetUInt16.h"
#include "DotNetTypes/UDotNetUInt32.h"
#include "DotNetTypes/UDotNetUInt64.h"
#include "DotNetTypes/UDotNetUInt8.h"
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
template<> struct TFormulaTypeMap<bool>			{ using Type = UDotNetBoolean; };
template<> struct TFormulaTypeMap<int8>			{ using Type = UDotNetInt8; };
template<> struct TFormulaTypeMap<int16>		{ using Type = UDotNetInt16; };
template<> struct TFormulaTypeMap<int32>		{ using Type = UDotNetInt32; };
template<> struct TFormulaTypeMap<int64>		{ using Type = UDotNetInt64; };
template<> struct TFormulaTypeMap<uint8>		{ using Type = UDotNetUInt8; };
template<> struct TFormulaTypeMap<uint16>		{ using Type = UDotNetUInt16; };
template<> struct TFormulaTypeMap<uint32>		{ using Type = UDotNetUInt32; };
template<> struct TFormulaTypeMap<uint64>		{ using Type = UDotNetUInt64; };
template<> struct TFormulaTypeMap<float>		{ using Type = UDotNetSingle; };
template<> struct TFormulaTypeMap<double>		{ using Type = UDotNetDouble; };
template<> struct TFormulaTypeMap<FString>		{ using Type = UDotNetString; };
template<> struct TFormulaTypeMap<FName>		{ using Type = UDotNetName; };
template<> struct TFormulaTypeMap<FText>		{ using Type = UDotNetText; };
template<> struct TFormulaTypeMap<FTimespan>	{ using Type = UDotNetTimeSpan; };
template<> struct TFormulaTypeMap<FDateTime>	{ using Type = UDotNetDateTime; };
template<> struct TFormulaTypeMap<UObject*>		{ using Type = UDotNetObject; };
template<> struct TFormulaTypeMap<nullptr_t>	{ using Type = UDotNetObject; };

template<typename T, typename = void>
struct TIsFormulaTypeMapped : std::false_type {};

template<typename T>
struct TIsFormulaTypeMapped<T, std::void_t<typename TFormulaTypeMap<T>::Type>> : std::true_type {};

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
		if (CastField<FDoubleProperty>(NumericProp)) { return EFormulaValueType::Double; }
		else if (CastField<FFloatProperty>(NumericProp)) { return EFormulaValueType::Float; }
		else if (CastField<FInt8Property>(NumericProp))	{ return EFormulaValueType::Int8; }
		else if (CastField<FInt16Property>(NumericProp)) { return EFormulaValueType::Int16; }
		else if (CastField<FIntProperty>(NumericProp)) { return EFormulaValueType::Int32; }
		else if (CastField<FInt64Property>(NumericProp)) { return EFormulaValueType::Int64; }
		else if (CastField<FByteProperty>(NumericProp)) { return EFormulaValueType::UInt8; }
		else if (CastField<FUInt16Property>(NumericProp)) { return EFormulaValueType::UInt16; }
		else if (CastField<FUInt32Property>(NumericProp)) { return EFormulaValueType::UInt32; }
		else if (CastField<FUInt64Property>(NumericProp)) { return EFormulaValueType::UInt64; }
		else { return EFormulaValueType::UInt64; } // fallback to universal type
	}
	else if (CastField<FObjectPropertyBase>(FieldType))
	{
		if (FieldType == UDotNetObject::GetNullLiteralProperty())
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
		const auto TimespanStruct =  UDotNetTimeSpan::GetLiteralProperty()->Struct;
		const auto DateTimeStruct = UDotNetDateTime::GetLiteralProperty()->Struct;

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