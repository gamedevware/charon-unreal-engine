// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "FFormulaExpression.h"

class CHARON_API FMemberExpression : public FFormulaExpression
{
private:
	mutable TSharedPtr<FFormulaTypeReference> TypeReference;
	mutable TSharedPtr<FFormulaTypeReference> TypeReferenceSkippingSelf;
	
public:
	const bool bUseNullPropagation;
	const TSharedPtr<FFormulaExpression> Expression;
	const FString RawMemberName;
	const FString MemberName;
	const TArray<TSharedPtr<FFormulaTypeReference>> TypeArguments;
	
	explicit FMemberExpression(const TSharedRef<FJsonObject>& ExpressionObj);
	explicit FMemberExpression(const TSharedPtr<FFormulaExpression>& Expression, const FString& RawMemberName,
	const TArray<TSharedPtr<FFormulaTypeReference>>& TypeArguments, const bool bUseNullPropagation);
	
	virtual bool IsNullPropagationEnabled() const override;

	virtual FFormulaExecutionResult Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const override;

	bool TryGetTypeReferenceAndMemberName(TSharedPtr<FFormulaTypeReference>& OutTypeReference, FString& OutMemberName) const;

	inline static EFormulaExpressionType Type = EFormulaExpressionType::MemberExpression;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
	virtual bool IsValid() const override;
	virtual void DebugPrintTo(FString& OutValue) const override;
private:
	bool TryGetTypeReference(TSharedPtr<FFormulaTypeReference>& OutTypeReference, bool bSkipSelf = false) const;
	bool TryGetGlobalMemberValue(TSharedPtr<FFormulaValue>& MemberValue, const FFormulaExecutionContext& Context) const;
	static void GetGlobalMemberNames(TSet<FString>& MemberNames, const FFormulaExecutionContext& Context);
};
