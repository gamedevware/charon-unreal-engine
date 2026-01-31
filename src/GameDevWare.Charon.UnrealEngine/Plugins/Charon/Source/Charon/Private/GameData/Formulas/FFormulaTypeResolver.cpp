// Copyright GameDevWare, Denis Zykov 2025

#include "GameData/Formulas/FFormulaTypeResolver.h"
#include "FFormulaEnumType.h"
#include "FFormulaUnrealType.h"
#include "FDotNetSurrogateType.h"
#include "FFormulaArrayType.h"
#include "FFormulaMapType.h"
#include "FFormulaSetType.h"
#include "FFormulaUnknownType.h"
#include "GameData/Formulas/FFormulaTypeReference.h"
#include "GameData/Formulas/IFormulaType.h"
#include "GameData/Formulas/EFormulaValueType.h"

static TMap<UPTRINT, TSharedRef<IFormulaType>> TypesByIdentity;
static FCriticalSection TypesByIdentityLock;

UPTRINT GetTypeIdentity(const FProperty* InProperty)
{
	check(InProperty);

	UPTRINT TypeIdentity;
	if (const FObjectProperty* ObjProp = CastField<FObjectProperty>(InProperty))
	{
		TypeIdentity = reinterpret_cast<UPTRINT>(ObjProp->PropertyClass.Get());
	}
	else if (const FStructProperty* StructProp = CastField<FStructProperty>(InProperty))
	{
		TypeIdentity = reinterpret_cast<UPTRINT>(StructProp->Struct.Get());
	}
	else if (const FEnumProperty* EnumProp = CastField<FEnumProperty>(InProperty))
	{
		TypeIdentity = reinterpret_cast<UPTRINT>(EnumProp->GetEnum()) + 
			GetTypeIdentity(EnumProp->GetUnderlyingProperty());
	}
	else if (const FArrayProperty* ArrayProp = CastField<FArrayProperty>(InProperty))
	{
		TypeIdentity = reinterpret_cast<UPTRINT>(ArrayProp->GetClass()) + 
			GetTypeIdentity(ArrayProp->Inner);
	}
	else if (const FSetProperty* SetProp = CastField<FSetProperty>(InProperty))
	{
		TypeIdentity = reinterpret_cast<UPTRINT>(SetProp->GetClass()) + 
			GetTypeIdentity(SetProp->GetElementProperty());
	}
	else if (const FMapProperty* MapProp = CastField<FMapProperty>(InProperty))
	{
		TypeIdentity = reinterpret_cast<UPTRINT>(MapProp->GetClass()) + 
			GetTypeIdentity(MapProp->GetKeyProperty()) + 
			GetTypeIdentity(MapProp->GetValueProperty());
	}
	else
	{
		// For primitives, the FFieldClass address is our unique ID
		TypeIdentity = reinterpret_cast<UPTRINT>(InProperty->GetClass());
	}

	return TypeIdentity;
}
UPTRINT GetTypeIdentity(const UClass* InClass)
{
	check(InClass);
	
	return reinterpret_cast<UPTRINT>(InClass);
}
UPTRINT GetTypeIdentity(const UScriptStruct* InStruct)
{
	check(InStruct);
	
	return reinterpret_cast<UPTRINT>(InStruct);
}
UPTRINT GetTypeIdentity(const UEnum* InEnum)
{
	check(InEnum);
	
	return reinterpret_cast<UPTRINT>(InEnum);
}

UClass* GetSurrogateType(const EFormulaValueType InTypeCode)
{
	switch (InTypeCode)
	{
	case EFormulaValueType::Boolean: return UDotNetBoolean::StaticClass();
	case EFormulaValueType::UInt8:  return UDotNetUInt8::StaticClass();
	case EFormulaValueType::UInt16: return UDotNetUInt16::StaticClass();
	case EFormulaValueType::UInt32: return UDotNetUInt32::StaticClass();
	case EFormulaValueType::UInt64: return UDotNetUInt64::StaticClass();
	case EFormulaValueType::Int8: return UDotNetInt8::StaticClass();
	case EFormulaValueType::Int16: return UDotNetInt16::StaticClass();
	case EFormulaValueType::Int32: return UDotNetInt32::StaticClass();
	case EFormulaValueType::Int64: return UDotNetInt64::StaticClass();
	case EFormulaValueType::Float: return UDotNetSingle::StaticClass();
	case EFormulaValueType::Double: return UDotNetDouble::StaticClass();
	case EFormulaValueType::Timespan: return UDotNetTimeSpan::StaticClass();
	case EFormulaValueType::DateTime: return UDotNetDateTime::StaticClass();
	case EFormulaValueType::String: return UDotNetString::StaticClass();
	case EFormulaValueType::Name: return UDotNetName::StaticClass();
	case EFormulaValueType::Text: return UDotNetText::StaticClass();
	case EFormulaValueType::Enum:
	case EFormulaValueType::Struct:
	case EFormulaValueType::ObjectPtr:
	case EFormulaValueType::Null:
	default: return nullptr;
	}
}
TSharedRef<IFormulaType> CreateFormulaType(const FProperty* InValueType)
{
	check(InValueType);

	// TODO cache created types, because they are static and not changing in any manner
	
	if (const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(InValueType))
	{
		return MakeShared<FFormulaUnrealType>(ObjectProperty->PropertyClass);
	}
	else if (const FStructProperty* StructProperty = CastField<FStructProperty>(InValueType))
	{
		return MakeShared<FFormulaUnrealType>(StructProperty->Struct);
	}
	else if (const FEnumProperty* EnumProperty = CastField<FEnumProperty>(InValueType))
	{
		const auto UnderlyingType = CreateFormulaType(EnumProperty->GetUnderlyingProperty());
		return MakeShared<FFormulaEnumType>(EnumProperty->GetEnum(), UnderlyingType);
	}
	else if (const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(InValueType))
	{
		const auto ElementType = CreateFormulaType(ArrayProperty->Inner);
		return MakeShared<FFormulaArrayType>(ElementType);
	}
	else if (const FSetProperty* SetProperty = CastField<FSetProperty>(InValueType))
	{
		const auto ElementType = CreateFormulaType(SetProperty->GetElementProperty());
		return MakeShared<FFormulaSetType>(ElementType);
	}
	else if (const FMapProperty* MapProperty = CastField<FMapProperty>(InValueType))
	{
		const auto KeyType = CreateFormulaType(MapProperty->GetKeyProperty());
		const auto ValueType = CreateFormulaType(MapProperty->GetValueProperty());
		return MakeShared<FFormulaMapType>(KeyType, ValueType);
	}
	else if (UClass* SurrogateClass = GetSurrogateType(GetPropertyTypeCode(InValueType)))
	{
		return MakeShared<FDotNetSurrogateType>(SurrogateClass);
	}
	
	return MakeShared<FFormulaUnknownType>(InValueType);
}

FFormulaTypeResolver::FFormulaTypeResolver(const TSharedPtr<FFormulaTypeResolver> Parent,
                                           const TArray<UObject*>& KnownTypes)
{
	this->Parent = Parent;

	FString FullName;
	FString Prefix;
	TSet<UObject*> ProcessedTypes;
	for (UObject* KnownType : KnownTypes)
	{
		if (ProcessedTypes.Contains(KnownType))
		{
			continue;; // already added
		}
		ProcessedTypes.Add(KnownType);

		Prefix.Reset();
		FullName.Reset();
		
		KnownType->GetName(FullName);
		TSharedPtr<IFormulaType> FormulaType;
		if (UClass* ClassPtr = Cast<UClass>(KnownType))
		{
			FormulaType = GetType(ClassPtr);
			Prefix = ClassPtr->GetPrefixCPP();
		}
		else if (UScriptStruct* StructPtr = Cast<UScriptStruct>(KnownType))
		{
			FormulaType = GetType(StructPtr);
			Prefix = StructPtr->GetPrefixCPP();
		}
		else if (UEnum* EnumPtr = Cast<UEnum>(KnownType))
		{
			FormulaType = GetType(EnumPtr);
			if (FullName.Len() > 2 && FullName.StartsWith("E"))
			{
				FullName.RemoveAt(0);
				Prefix = TEXT("E");
			}
		}

		if (!FormulaType.IsValid())
		{
			continue;
		}
		
		this->TypesByName.Add(FullName, FormulaType);
		if (Prefix.Len() > 0)
		{
			Prefix.Append(FullName);
			this->TypesByName.Add(Prefix, FormulaType);
		}
	}
}

TMap<FString, UClass*>& GetOrCreateBuildInTypes()
{
	static TMap<FString, UClass*> BuildInTypes = {
		// basic types
		{  TEXT("bool"), UDotNetBoolean::StaticClass() },
		{  TEXT("Boolean"), UDotNetBoolean::StaticClass() },
		{  TEXT("System.Boolean"), UDotNetBoolean::StaticClass() },
		{  TEXT("char"), UDotNetUInt16::StaticClass() },
		{  TEXT("Char"), UDotNetUInt16::StaticClass() },
		{  TEXT("System.Char"), UDotNetUInt16::StaticClass() },
		{  TEXT("int8"), UDotNetInt8::StaticClass() },
		{  TEXT("sbyte"), UDotNetInt8::StaticClass() },
		{  TEXT("SByte"), UDotNetInt8::StaticClass() },
		{  TEXT("System.SByte"), UDotNetInt8::StaticClass() },
		{  TEXT("int16"), UDotNetInt16::StaticClass() },
		{  TEXT("short"), UDotNetInt16::StaticClass() },
		{  TEXT("Int16"), UDotNetInt16::StaticClass() },
		{  TEXT("System.Int16"), UDotNetInt16::StaticClass() },
		{  TEXT("int32"), UDotNetInt32::StaticClass() },
		{  TEXT("int"), UDotNetInt32::StaticClass() },
		{  TEXT("Int32"), UDotNetInt32::StaticClass() },
		{  TEXT("System.Int32"), UDotNetInt32::StaticClass() },
		{  TEXT("int64"), UDotNetInt64::StaticClass() },
		{  TEXT("long"), UDotNetInt64::StaticClass() },
		{  TEXT("Int64"), UDotNetInt64::StaticClass() },
		{  TEXT("System.Int64"), UDotNetInt64::StaticClass() },
		{  TEXT("uint8"), UDotNetUInt8::StaticClass() },
		{  TEXT("byte"), UDotNetUInt8::StaticClass() },
		{  TEXT("Byte"), UDotNetUInt8::StaticClass() },
		{  TEXT("System.Byte"), UDotNetUInt8::StaticClass() },
		{  TEXT("uint16"), UDotNetUInt16::StaticClass() },
		{  TEXT("ushort"), UDotNetUInt16::StaticClass() },
		{  TEXT("UInt16"), UDotNetUInt16::StaticClass() },
		{  TEXT("System.UInt16"), UDotNetUInt16::StaticClass() },
		{  TEXT("uint32"), UDotNetUInt32::StaticClass() },
		{  TEXT("uint"), UDotNetUInt32::StaticClass() },
		{  TEXT("UInt32"), UDotNetUInt32::StaticClass() },
		{  TEXT("System.UInt32"), UDotNetUInt32::StaticClass() },
		{  TEXT("uint64"), UDotNetUInt64::StaticClass() },
		{  TEXT("ulong"), UDotNetUInt64::StaticClass() },
		{  TEXT("UInt64"), UDotNetUInt64::StaticClass() },
		{  TEXT("System.UInt64"), UDotNetUInt64::StaticClass() },
		{  TEXT("float"), UDotNetSingle::StaticClass() },
		{  TEXT("Single"), UDotNetSingle::StaticClass() },
		{  TEXT("System.Single"), UDotNetSingle::StaticClass() },
		{  TEXT("double"), UDotNetDouble::StaticClass() },
		{  TEXT("Double"), UDotNetDouble::StaticClass() },
		{  TEXT("System.Double"), UDotNetDouble::StaticClass() },
		{  TEXT("FString"), UDotNetString::StaticClass() },
		{  TEXT("string"), UDotNetString::StaticClass() },
		{  TEXT("String"), UDotNetString::StaticClass() },
		{  TEXT("System.String"), UDotNetString::StaticClass() },
		{  TEXT("FText"), UDotNetText::StaticClass() },
		{  TEXT("Text"), UDotNetText::StaticClass() },
		{  TEXT("FName"), UDotNetName::StaticClass() },
		{  TEXT("Name"), UDotNetName::StaticClass() },
		{  TEXT("nullptr_t"), UDotNetObject::StaticClass() },
		{  TEXT("UObject"), UDotNetObject::StaticClass() },
		{  TEXT("object"), UDotNetObject::StaticClass() },
		{  TEXT("Object"), UDotNetObject::StaticClass() },
		{  TEXT("System.Object"), UDotNetObject::StaticClass() },
		
		// structures
		{  TEXT("DateTime"), UDotNetDateTime::StaticClass() },
		{  TEXT("System.DateTime"), UDotNetDateTime::StaticClass() },
		{  TEXT("TimeSpan"), UDotNetTimeSpan::StaticClass() },
		{  TEXT("System.TimeSpan"), UDotNetTimeSpan::StaticClass() },

		// helper types
		// {  TEXT("Math"), UDotNetObject::StaticClass() },
		// {  TEXT("System.Math"), UDotNetObject::StaticClass() },
		// {  TEXT("Enum"), UDotNetObject::StaticClass() },
		// {  TEXT("System.Enum"), UDotNetObject::StaticClass() },
		// {  TEXT("DayOfWeek"), UDotNetObject::StaticClass() },
		// {  TEXT("System.DayOfWeek"), UDotNetObject::StaticClass() },
		// {  TEXT("StringComparison"), UDotNetObject::StaticClass() },
		// {  TEXT("System.StringComparison"), UDotNetObject::StaticClass() },
		// {  TEXT("StringSplitOptions"), UDotNetObject::StaticClass() },
		// {  TEXT("System.StringSplitOptions"), UDotNetObject::StaticClass() },
	};
	return BuildInTypes;
}

TSharedPtr<IFormulaType> FFormulaTypeResolver::FindType(const TSharedPtr<FFormulaTypeReference>& TypeReference)
{
	if (!TypeReference.IsValid())
	{
		return nullptr;
	}

	
	const FString FullName = TypeReference->GetFullName(false);
	if (auto* FormulaType = this->TypesByName.Find(FullName); FormulaType)
	{
		return *FormulaType;
	}
	else if (auto* BuildInType = GetOrCreateBuildInTypes().Find(FullName); BuildInType)
	{
		return GetType(*BuildInType);
	}

	if (Parent.IsValid())
	{
		return Parent->FindType(TypeReference);
	}
	return nullptr;
}

TSharedRef<IFormulaType> FFormulaTypeResolver::GetType(const TSharedRef<FFormulaValue>& InValue)
{
	void* ContainerPtr = nullptr;
	if (InValue->GetTypeCode() == EFormulaValueType::ObjectPtr && 
		InValue->TryGetContainerAddress(ContainerPtr))
	{
		check(ContainerPtr);
		/** * Promote UObject values to their specific leaf class.
			*
			* NOTE: This introduces dynamic-dispatch behavior. While this deviates from 
			* strict static typing, it is necessary because FFormulaValue uses type erasure 
			* to support arbitrary UObjects beyond those restricted to FProperty storage.
			*/
		return GetType(static_cast<UObject*>(ContainerPtr)->GetClass());
	}
	else
	{
		return GetType(InValue->GetType());
	}
}

TSharedRef<IFormulaType> FFormulaTypeResolver::GetType(FProperty* InProperty)
{
	check(InProperty);

	const auto TypeKey = GetTypeIdentity(InProperty);
	{
		FScopeLock ReadLock(&TypesByIdentityLock);
		if (TSharedRef<IFormulaType>* CachedType = TypesByIdentity.Find(TypeKey))
		{
			return *CachedType;
		}
	}
	{
		FScopeLock WriteLock(&TypesByIdentityLock);
		return TypesByIdentity.Add(TypeKey, CreateFormulaType(InProperty));
	}
}

TSharedRef<IFormulaType> FFormulaTypeResolver::GetType(UClass* InClass)
{
	check(InClass);
	
	const auto TypeKey = GetTypeIdentity(InClass);
	{
		FScopeLock ReadLock(&TypesByIdentityLock);
		if (TSharedRef<IFormulaType>* CachedType = TypesByIdentity.Find(TypeKey))
		{
			return *CachedType;
		}
	}
	{
		FScopeLock WriteLock(&TypesByIdentityLock);
		if (InClass->HasAllClassFlags(EClassFlags::CLASS_Hidden | EClassFlags::CLASS_Abstract) &&
			InClass->GetName().StartsWith("DotNet"))
		{
			return TypesByIdentity.Add(TypeKey, MakeShared<FDotNetSurrogateType>(InClass));
		}
		else
		{
			return TypesByIdentity.Add(TypeKey, MakeShared<FFormulaUnrealType>(InClass));
		}
	}
}

TSharedRef<IFormulaType> FFormulaTypeResolver::GetType(UScriptStruct* InStruct)
{
	check(InStruct);
	
	const auto TypeKey = GetTypeIdentity(InStruct);
	{
		FScopeLock ReadLock(&TypesByIdentityLock);
		if (TSharedRef<IFormulaType>* CachedType = TypesByIdentity.Find(TypeKey))
		{
			return *CachedType;
		}
	}
	{
		FScopeLock WriteLock(&TypesByIdentityLock);
		return TypesByIdentity.Add(TypeKey, MakeShared<FFormulaUnrealType>(InStruct));
	}
}

TSharedRef<IFormulaType> FFormulaTypeResolver::GetType(UEnum* InEnum)
{
	check(InEnum);

	const auto TypeKey = GetTypeIdentity(InEnum);
	{
		FScopeLock ReadLock(&TypesByIdentityLock);
		if (TSharedRef<IFormulaType>* CachedType = TypesByIdentity.Find(TypeKey))
		{
			return *CachedType;
		}
	}
	{
		FScopeLock WriteLock(&TypesByIdentityLock);
		TSharedRef<IFormulaType> UnderlyingType = GetType(UDotNetInt64::StaticClass()); // not correct, because in 99% it is int8
		return TypesByIdentity.Add(TypeKey, MakeShared<FFormulaEnumType>(InEnum, UnderlyingType));
	}
}
