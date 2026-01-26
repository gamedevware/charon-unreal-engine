// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "FFormulaExpression.h"

class CHARON_API FIndexExpression : public FFormulaExpression
{
public:
	const TMap<FString, TSharedPtr<FFormulaExpression>> Arguments;
	const TSharedPtr<FFormulaExpression> Expression;
	const bool bUseNullPropagation;

	explicit FIndexExpression(const TSharedRef<FJsonObject>& ExpressionObj);
	explicit FIndexExpression(const TSharedPtr<FFormulaExpression>& Expression, const TMap<FString, TSharedPtr<FFormulaExpression>>& Arguments, bool bUseNullPropagation);
	
	virtual bool IsNullPropagationEnabled() const override;

	virtual FFormulaExecutionResult Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const override;

	inline static EFormulaExpressionType Type = EFormulaExpressionType::IndexExpression;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
	virtual bool IsValid() const override;
	virtual void DebugPrintTo(FString& OutValue) const override;
};