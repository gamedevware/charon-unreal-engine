#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "Containers/Array.h"
#include "Templates/SharedPointer.h"
#include "Dom/JsonObject.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFormulaTypeReference, Log, All);

class CHARON_API FFormulaTypeReference
{
public:
	TSharedPtr<FFormulaTypeReference> Expression;
	TArray<TSharedPtr<FFormulaTypeReference>> TypeArguments;
	FString Name;
	// Cache variables for lazy initialization
	mutable FString FullName;

	explicit FFormulaTypeReference(const TSharedRef<FJsonObject>& ExpressionObj);
	explicit FFormulaTypeReference(FString Name);

	bool IsEmpty() const;

	FString GetFullName(bool bWriteGenerics) const;

private:
	void BuildFullNameInternal(FStringBuilderBase& Builder, bool bWriteGenerics) const;
};
