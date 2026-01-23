#include "GameData/Formulas/FFormulaTypeResolver.h"

#include "FFormulaEnumDescription.h"
#include "FFormulaUnrealType.h"
#include "FDotNetSurrogateType.h"
#include "GameData/Formulas/FFormulaTypeReference.h"
#include "GameData/Formulas/IFormulaTypeDescription.h"
#include "GameData/Formulas/EFormulaValueType.h"

TSharedPtr<IFormulaTypeDescription> CreateTypeDescription(const FProperty* Type)
{
	check(Type);

	// TODO cache created types, because they are static and not changing in any manner
	
	if (const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(Type))
	{
		return MakeShared<FFormulaUnrealType>(ObjectProperty->PropertyClass);
	}
	else if (const FStructProperty* StructProperty = CastField<FStructProperty>(Type))
	{
		return MakeShared<FFormulaUnrealType>(StructProperty->Struct);
	}
	else if (const FEnumProperty* EnumProperty = CastField<FEnumProperty>(Type))
	{
		return MakeShared<FFormulaEnumDescription>(EnumProperty->GetEnum());
	}

	switch (GetPropertyTypeCode(Type))
	{
	case EFormulaValueType::Boolean:
	case EFormulaValueType::UInt8:
	case EFormulaValueType::UInt16:
	case EFormulaValueType::UInt32:
	case EFormulaValueType::UInt64:
	case EFormulaValueType::Int8:
	case EFormulaValueType::Int16:
	case EFormulaValueType::Int32:
	case EFormulaValueType::Int64:
	case EFormulaValueType::Float:
	case EFormulaValueType::Double:
	case EFormulaValueType::Timespan:
	case EFormulaValueType::DateTime:
	case EFormulaValueType::String:
	case EFormulaValueType::Name:
	case EFormulaValueType::Text:
	case EFormulaValueType::Enum:
	case EFormulaValueType::Struct:
	case EFormulaValueType::ObjectPtr:
	case EFormulaValueType::Null:
	default:
		return nullptr;
	}
}

FFormulaTypeResolver::FFormulaTypeResolver(const TSharedPtr<FFormulaTypeResolver> Parent,
                                           const TArray<UObject*>& KnownTypes)
{
	this->Parent = Parent;

	TSet<UObject*> ProcessedTypes;
	for (UObject* KnownType : KnownTypes)
	{
		if (ProcessedTypes.Contains(KnownType))
		{
			continue;; // already added
		}
		ProcessedTypes.Add(KnownType);

		if (UClass* KnownClassPtr = Cast<UClass>(KnownType))
		{
			FString FullName = KnownClassPtr->GetName();
			FString PrefixedFullName = KnownClassPtr->GetPrefixCPP() + KnownClassPtr->GetName();
			TSharedPtr<IFormulaTypeDescription> TypeDescription = MakeShared<FFormulaUnrealType>(KnownClassPtr);

			this->TypesByName.Add(FullName, TypeDescription);
			if (FullName != PrefixedFullName)
			{
				this->TypesByName.Add(PrefixedFullName, TypeDescription);
			}
		}
	}
}

TSharedPtr<IFormulaTypeDescription> FFormulaTypeResolver::GetTypeDescription(const TSharedPtr<FFormulaTypeReference>& TypeReference)
{
	if (!TypeReference.IsValid())
	{
		return nullptr;
	}

	const FString FullName = TypeReference->GetFullName(false);
	if (auto* TypeDescription = this->TypesByName.Find(FullName))
	{
		return *TypeDescription;
	}

	return Parent->GetTypeDescription(TypeReference);
}

TSharedPtr<IFormulaTypeDescription> FFormulaTypeResolver::GetTypeDescription(const FProperty* Type)
{
	check(Type);
	
	const FString CppName = Type->GetNameCPP();
	if (auto* TypeDescription = this->TypesByName.Find(CppName))
	{
		return *TypeDescription;
	}
	
	return this->TypesByName.Add(CppName, CreateTypeDescription(Type));
}