#include "GameData/Formulas/FFormulaTypeResolver.h"
#include "GameData/Formulas/FFormulaTypeReference.h"
#include "GameData/Formulas/FFormulaVariableValue.h"

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

static TMap<FString, EFormulaVariableType> PrimitivePropertyTypes = {
	{TEXT("int"), EFormulaVariableType::Int32},
	{TEXT("int32"), EFormulaVariableType::Int32},
	{TEXT("int32_t"), EFormulaVariableType::Int32},
	{TEXT("int64"), EFormulaVariableType::Int64},
	{TEXT("int64_t"), EFormulaVariableType::Int64},
	{TEXT("int16"), EFormulaVariableType::Int32},
	{TEXT("int16_t"), EFormulaVariableType::Int32},
	{TEXT("int8"), EFormulaVariableType::Int32},
	{TEXT("int8_t"), EFormulaVariableType::Int32},
	{TEXT("byte"), EFormulaVariableType::UInt32},
	{TEXT("uint8"), EFormulaVariableType::UInt32},
	{TEXT("uint32"), EFormulaVariableType::UInt32},
	{TEXT("uint64"), EFormulaVariableType::UInt64},

	{TEXT("float"), EFormulaVariableType::Float},
	{TEXT("float32"), EFormulaVariableType::Float},
	{TEXT("double"), EFormulaVariableType::Double},
	{TEXT("float64"), EFormulaVariableType::Double},

	{TEXT("bool"), EFormulaVariableType::Bool},
	{TEXT("boolean"), EFormulaVariableType::Bool},

	{TEXT("fstring"), EFormulaVariableType::FString},
	{TEXT("string"), EFormulaVariableType::FString},
	{TEXT("fname"), EFormulaVariableType::FName},
	{TEXT("ftext"), EFormulaVariableType::FText},
};

EFormulaVariableType GetPropertyType(TSharedPtr<FFormulaTypeReference> TypeReference)
{
	if (!TypeReference.IsValid())
	{
		return EFormulaVariableType::Null;
	}

	// Get the name and convert to lowercase for case-insensitive lookup
	const FString FullName = TypeReference->GetFullName(false).ToLower();

	// Look for the string in our primitive map
	if (const EFormulaVariableType* FoundType = PrimitivePropertyTypes.Find(FullName))
	{
		return *FoundType;
	}

	// Default fallback: If it's not a primitive, it's likely a complex type or invalid
	return EFormulaVariableType::Null;
}
