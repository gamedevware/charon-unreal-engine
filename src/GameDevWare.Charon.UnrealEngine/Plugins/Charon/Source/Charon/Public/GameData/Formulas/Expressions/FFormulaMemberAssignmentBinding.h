// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "FFormulaMemberBinding.h"

class FFormulaExpression;

class CHARON_API FFormulaMemberAssignmentBinding : public FFormulaMemberBinding
{
public:
	TSharedPtr<FFormulaExpression> const Expression;
	
	explicit FFormulaMemberAssignmentBinding(const TSharedRef<FJsonObject>& ExpressionObj);
	explicit FFormulaMemberAssignmentBinding(const FString& RawMemberName, const TSharedPtr<FFormulaExpression>& Expression);

	inline static EFormulaExpressionType Type = EFormulaExpressionType::MemberAssignmentBinding;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
	virtual bool IsValid() const override;
	
	virtual FFormulaExecutionResult ApplyToMember(const TSharedRef<FFormulaValue>& Target, const FFormulaProperty* Member, const FFormulaExecutionContext& Context) const override;

	virtual void DebugPrintTo(FString& OutValue) const override;
};
