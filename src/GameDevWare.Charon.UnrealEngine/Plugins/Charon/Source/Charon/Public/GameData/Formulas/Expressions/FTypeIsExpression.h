// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "FFormulaExpression.h"
#include "../FFormulaTypeReference.h"

class CHARON_API FTypeIsExpression final : public FFormulaExpression
{
public:
	const TSharedPtr<FFormulaTypeReference> CheckType;
	const TSharedPtr<FFormulaExpression> Expression;
	
	explicit FTypeIsExpression(const TSharedRef<FJsonObject>& ExpressionObj);
	explicit FTypeIsExpression(const TSharedPtr<FFormulaExpression>& Expression, const TSharedPtr<FFormulaTypeReference>& CheckType);

	virtual FFormulaExecutionResult Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const override; 
	
	inline static EFormulaExpressionType Type = EFormulaExpressionType::TypeIsExpression;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
	virtual bool IsValid() const override;
	virtual void DebugPrintTo(FString& OutValue) const override;
};
