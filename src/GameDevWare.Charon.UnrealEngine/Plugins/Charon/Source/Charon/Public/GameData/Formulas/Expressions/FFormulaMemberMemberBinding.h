// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "FFormulaMemberBinding.h"

class FFormulaExpression;

class CHARON_API FFormulaMemberMemberBinding : public FFormulaMemberBinding
{
public:
	const TArray<TSharedPtr<FFormulaMemberBinding>> Bindings;
	
	explicit FFormulaMemberMemberBinding(const TSharedRef<FJsonObject>& ExpressionObj);
	explicit FFormulaMemberMemberBinding(const FString& RawMemberName, const TArray<TSharedPtr<FFormulaMemberBinding>>& Bindings);

	inline static EFormulaExpressionType Type = EFormulaExpressionType::MemberMemberBinding;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
	virtual bool IsValid() const override;
	
	virtual FFormulaExecutionResult ApplyToMember(const TSharedRef<FFormulaValue>& Target, const FFormulaProperty* Member, const FFormulaExecutionContext& Context) const override;

	virtual void DebugPrintTo(FString& OutValue) const override;
};
