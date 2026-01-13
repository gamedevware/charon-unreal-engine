#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "Containers/Array.h"
#include "Templates/SharedPointer.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/Class.h"

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
};

