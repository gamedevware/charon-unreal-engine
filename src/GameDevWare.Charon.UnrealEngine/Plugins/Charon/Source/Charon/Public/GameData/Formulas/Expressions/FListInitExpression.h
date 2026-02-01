// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "FFormulaExpression.h"

DECLARE_LOG_CATEGORY_EXTERN(LogListInitExpression, Log, All);

class FNewExpression;
class FFormulaElementInitBinding;

class CHARON_API FListInitExpression : public FFormulaExpression
{
public:
	const TSharedPtr<FNewExpression> NewExpression;
	const TArray<TSharedPtr<FFormulaElementInitBinding>> Initializers;
	
	explicit FListInitExpression(const TSharedRef<FJsonObject>& ExpressionObj);
	explicit FListInitExpression(const TSharedPtr<FNewExpression>& NewExpression, const TArray<TSharedPtr<FFormulaElementInitBinding>>& Initializers);

	virtual FFormulaExecutionResult Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const override;

	inline static EFormulaExpressionType Type = EFormulaExpressionType::ListInitExpression;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
	virtual bool IsValid() const override;
	virtual void DebugPrintTo(FString& OutValue) const override;
};
