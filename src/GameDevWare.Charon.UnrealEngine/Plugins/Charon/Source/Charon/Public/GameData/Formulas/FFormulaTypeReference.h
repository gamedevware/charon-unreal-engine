// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "Containers/Array.h"
#include "Templates/SharedPointer.h"
#include "Dom/JsonObject.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFormulaTypeReference, Log, All);

class CHARON_API FFormulaTypeReference
{
private:
	// Cache variables for lazy initialization
	mutable FString FullName;
	mutable FString FullNameWithGenerics;
public:
	const TSharedPtr<FFormulaTypeReference> Expression;
	const TArray<TSharedPtr<FFormulaTypeReference>> TypeArguments;
	const FString Name;

	explicit FFormulaTypeReference(const TSharedRef<FJsonObject>& ExpressionObj);
	explicit FFormulaTypeReference(const FString& Name);
	explicit FFormulaTypeReference(const FString& Name, const TArray<TSharedPtr<FFormulaTypeReference>>& TypeArguments);
	explicit FFormulaTypeReference(const FString& Name, const TSharedPtr<FFormulaTypeReference>& Expression, const TArray<TSharedPtr<FFormulaTypeReference>>& TypeArguments);

	bool IsEmpty() const;

	FString GetFullName(bool bWriteGenerics) const;

private:
	void BuildFullNameInternal(FStringBuilderBase& Builder, bool bWriteGenerics) const;
};
