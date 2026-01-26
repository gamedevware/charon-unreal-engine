// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "FFormulaExpression.h"

class CHARON_API FLambdaExpression : public FFormulaExpression
{
public:
	const TSharedPtr<FFormulaExpression> Body;
	const TArray<TPair<FString, TSharedPtr<FFormulaTypeReference>>> Arguments;
	
	explicit FLambdaExpression(const TSharedRef<FJsonObject>& ExpressionObj);

	virtual FFormulaExecutionResult Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const override;
	
	inline static EFormulaExpressionType Type = EFormulaExpressionType::LambdaExpression;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
	virtual bool IsValid() const override;
	virtual void DebugPrintTo(FString& OutValue) const override;
};
