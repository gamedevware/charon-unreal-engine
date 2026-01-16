#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "Containers/Array.h"
#include "Templates/SharedPointer.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/Class.h"

class IFormulaTypeDescription;
class FFormulaTypeReference;

class CHARON_API FFormulaTypeResolver : public TSharedFromThis<FFormulaTypeResolver>
{
	TSharedPtr<FFormulaTypeResolver> Parent;
	TMap<FString, TSharedPtr<IFormulaTypeDescription>> TypesByName;
	
public:
	FFormulaTypeResolver(const TSharedPtr<FFormulaTypeResolver> Parent, const TArray<UObject*>& KnownTypes);
	virtual ~FFormulaTypeResolver() = default;

	TSharedPtr<IFormulaTypeDescription> GetTypeDescription(const TSharedPtr<FFormulaTypeReference>& TypeReference);
};
