#pragma once

DECLARE_LOG_CATEGORY_EXTERN(LogFormulaTypeReference, Log, All);

class CHARON_API FFormulaTypeReference
{
private:
	TSharedPtr<FFormulaTypeReference> Expression;
	TArray<TSharedPtr<FFormulaTypeReference>> TypeArguments;
	FString Name;
	// Cache variables for lazy initialization
	mutable FString FullName;
public:
	explicit FFormulaTypeReference(const TSharedRef<FJsonObject>& ExpressionObj);
	explicit FFormulaTypeReference(FString Name);

	bool IsEmpty() const;

	FString GetFullName(bool bWriteGenerics) const;

private:
	void BuildFullNameInternal(FStringBuilderBase& Builder, bool bWriteGenerics) const;
};
