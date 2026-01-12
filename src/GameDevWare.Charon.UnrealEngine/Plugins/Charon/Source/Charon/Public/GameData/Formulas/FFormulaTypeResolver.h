#pragma once

class FFormulaTypeReference;

class CHARON_API FFormulaTypeResolver : public TSharedFromThis<FFormulaTypeResolver>
{
	TSharedPtr<FFormulaTypeResolver> Parent;
	TArray<TStrongObjectPtr<UObject>> KnownTypes;
	
public:
	FFormulaTypeResolver(const TSharedPtr<FFormulaTypeResolver> Parent, const TArray<UObject*>& KnownTypes);
	virtual ~FFormulaTypeResolver() = default;

	UClass* GetClass(TSharedPtr<FFormulaTypeReference> TypeReference);
	UEnum* GetEnum(TSharedPtr<FFormulaTypeReference> TypeReference);
	EPropertyType GetPropertyType(TSharedPtr<FFormulaTypeReference> TypeReference);
};

