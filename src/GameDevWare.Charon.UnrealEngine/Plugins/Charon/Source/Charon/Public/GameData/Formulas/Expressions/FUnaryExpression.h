// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "EUnaryOperationType.h"
#include "FFormulaExpression.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUnaryExpression, Log, All);

class CHARON_API FUnaryExpression : public FFormulaExpression
{
public:
	const TSharedPtr<FFormulaExpression> Expression;
	const EUnaryOperationType UnaryOperationType;
	
	explicit FUnaryExpression(const TSharedPtr<FJsonObject>& ExpressionObj);
	explicit FUnaryExpression(const TSharedPtr<FFormulaExpression>& Expression, EUnaryOperationType UnaryOperationType);

	virtual FFormulaExecutionResult Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const override;
	
	inline static EFormulaExpressionType Type = EFormulaExpressionType::UnaryExpression;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
	virtual bool IsValid() const override;
	virtual void DebugPrintTo(FString& OutValue) const override;
};