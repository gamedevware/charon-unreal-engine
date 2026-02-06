// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "FFormulaMemberBinding.h"

class FFormulaElementInitBinding;

class CHARON_API FFormulaMemberListBinding : public FFormulaMemberBinding
{
public:
	const TArray<TSharedPtr<FFormulaElementInitBinding>> Initializers;
	
	explicit FFormulaMemberListBinding(const TSharedRef<FJsonObject>& ExpressionObj);
	explicit FFormulaMemberListBinding(const FString& RawMemberName, const TArray<TSharedPtr<FFormulaElementInitBinding>>& Initializers);

	inline static EFormulaExpressionType Type = EFormulaExpressionType::MemberListBinding;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
	virtual bool IsValid() const override;
	
	virtual FFormulaExecutionResult ApplyToMember(const TSharedRef<FFormulaValue>& Target, const FFormulaProperty* Member, const FFormulaExecutionContext& Context) const override;

	virtual void DebugPrintTo(FString& OutValue) const override;
};
