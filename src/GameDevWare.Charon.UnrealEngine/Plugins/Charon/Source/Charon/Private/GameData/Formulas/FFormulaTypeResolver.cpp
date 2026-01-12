#include "GameData/Formulas/FFormulaTypeResolver.h"
#include "GameData/Formulas/FFormulaTypeReference.h"

FFormulaTypeResolver::FFormulaTypeResolver(const TSharedPtr<FFormulaTypeResolver> Parent,
                                           const TArray<UObject*>& KnownTypes)
{
	this->Parent = Parent;

	for (UObject* KnownType : KnownTypes)
	{
		if (!KnownType)
		{
			continue;
		}
		this->KnownTypes.Add(TStrongObjectPtr(KnownType));
	}
}

UClass* FFormulaTypeResolver::GetClass(TSharedPtr<FFormulaTypeReference> TypeReference)
{
	if (!TypeReference.IsValid())
	{
		return nullptr;
	}

	const FString FullName = TypeReference->GetFullName(false);
	for (auto KnownType : KnownTypes)
	{
		UClass* KnownTypeClass = Cast<UClass>(KnownType.Get());
		if (KnownTypeClass && (KnownTypeClass->GetName() == FullName ||
			KnownTypeClass->GetPrefixCPP() + KnownTypeClass->GetName() == FullName))
		{
			return KnownTypeClass;
		}
	}
	return Parent->GetClass(TypeReference);
}

UEnum* FFormulaTypeResolver::GetEnum(TSharedPtr<FFormulaTypeReference> TypeReference)
{
	if (!TypeReference.IsValid())
	{
		return nullptr;
	}

	const FString FullName = TypeReference->GetFullName(false);
	for (auto KnownType : KnownTypes)
	{
		UEnum* KnownTypeEnum = Cast<UEnum>(KnownType.Get());
		if (KnownTypeEnum && (KnownTypeEnum->GetName() == FullName ||
			TEXT("E") + KnownTypeEnum->GetName() == FullName))
		{
			return KnownTypeEnum;
		}
	}
	return Parent->GetEnum(TypeReference);
}

static TMap<FString, EPropertyType> PrimitivePropertyTypes = {
	{TEXT("int"), EPropertyType::CPT_Int},
	{TEXT("int32"), EPropertyType::CPT_Int},
	{TEXT("int32_t"), EPropertyType::CPT_Int},
	{TEXT("int64"), EPropertyType::CPT_Int64},
	{TEXT("int64_t"), EPropertyType::CPT_Int64},
	{TEXT("int16"), EPropertyType::CPT_Int16},
	{TEXT("int16_t"), EPropertyType::CPT_Int16},
	{TEXT("int8"), EPropertyType::CPT_Int8},
	{TEXT("int8_t"), EPropertyType::CPT_Int8},
	{TEXT("byte"), EPropertyType::CPT_Byte},
	{TEXT("uint8"), EPropertyType::CPT_Byte},
	{TEXT("uint32"), EPropertyType::CPT_UInt32},
	{TEXT("uint64"), EPropertyType::CPT_UInt64},

	{TEXT("float"), EPropertyType::CPT_Float},
	{TEXT("float32"), EPropertyType::CPT_Float},
	{TEXT("double"), EPropertyType::CPT_Double},
	{TEXT("float64"), EPropertyType::CPT_Double},

	{TEXT("bool"), EPropertyType::CPT_Bool},
	{TEXT("boolean"), EPropertyType::CPT_Bool},

	{TEXT("fstring"), EPropertyType::CPT_String},
	{TEXT("string"), EPropertyType::CPT_String},
	{TEXT("fname"), EPropertyType::CPT_Name},
	{TEXT("ftext"), EPropertyType::CPT_Text},
};

EPropertyType FFormulaTypeResolver::GetPropertyType(TSharedPtr<FFormulaTypeReference> TypeReference)
{
	if (!TypeReference.IsValid())
	{
		return EPropertyType::CPT_None;
	}

	// Get the name and convert to lowercase for case-insensitive lookup
	const FString FullName = TypeReference->GetFullName(false).ToLower();

	// Look for the string in our primitive map
	if (const EPropertyType* FoundType = PrimitivePropertyTypes.Find(FullName))
	{
		return *FoundType;
	}

	// Default fallback: If it's not a primitive, it's likely a complex type or invalid
	return EPropertyType::CPT_None;
}
