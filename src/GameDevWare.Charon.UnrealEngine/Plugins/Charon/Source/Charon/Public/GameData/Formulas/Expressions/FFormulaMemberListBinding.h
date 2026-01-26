// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "FFormulaMemberBinding.h"

class FFormulaExpression;

class FFormulaMemberListBinding : public FFormulaMemberBinding
{
public:
	const TArray<TSharedPtr<FFormulaExpression>> Initializers;
	
	explicit FFormulaMemberListBinding(const TSharedRef<FJsonObject>& ExpressionObj);
	explicit FFormulaMemberListBinding(const FString& RawMemberName, const  TArray<TSharedPtr<FFormulaExpression>>& Initializers);

	inline static EFormulaExpressionType Type = EFormulaExpressionType::MemberListBinding;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
	virtual bool IsValid() const override;
	
	virtual FFormulaExecutionResult ApplyMemberChanges(const TSharedRef<FFormulaValue>& Target, const FFormulaProperty* Member, const FFormulaExecutionContext& Context) const override;

	virtual void DebugPrintTo(FString& OutValue) const override;
};
