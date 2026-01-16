#include "GameData/Formulas/FFormulaTypeResolver.h"
#include "FFormulaUClassDescription.h"
#include "GameData/Formulas/FFormulaTypeReference.h"
#include "GameData/Formulas/IFormulaTypeDescription.h"

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
			TSharedPtr<IFormulaTypeDescription> TypeDescription = MakeShared<FFormulaUClassDescription>(KnownClassPtr);

			this->TypesByName.Add(FullName, TypeDescription);
			if (FullName != PrefixedFullName)
			{
				this->TypesByName.Add(PrefixedFullName, TypeDescription);
			}
		}
	}
}

TSharedPtr<IFormulaTypeDescription> FFormulaTypeResolver::GetTypeDescription(
	const TSharedPtr<FFormulaTypeReference>& TypeReference)
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
