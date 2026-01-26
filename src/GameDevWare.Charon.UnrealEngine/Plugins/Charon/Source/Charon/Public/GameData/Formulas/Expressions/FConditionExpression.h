// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "FFormulaExpression.h"

class CHARON_API FConditionExpression : public FFormulaExpression
{

public:
	const TSharedPtr<FFormulaExpression> Test;
	const TSharedPtr<FFormulaExpression> IfTrue;
	const TSharedPtr<FFormulaExpression> IfFalse;
	
	explicit FConditionExpression(const TSharedRef<FJsonObject>& ExpressionObj);
	explicit FConditionExpression(const TSharedPtr<FFormulaExpression>& Test, const TSharedPtr<FFormulaExpression>& IfTrue, const TSharedPtr<FFormulaExpression>& IfFalse);

	virtual FFormulaExecutionResult Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const override;

	inline static EFormulaExpressionType Type = EFormulaExpressionType::ConditionExpression;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
	virtual bool IsValid() const override;
	virtual void DebugPrintTo(FString& OutValue) const override;
};
