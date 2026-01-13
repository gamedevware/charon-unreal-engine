#pragma once

#include "FFormulaExpression.h"

class CHARON_API FInvokeExpression : public FFormulaExpression
{
private:
	TMap<FString, TSharedPtr<FFormulaExpression>> Arguments;
	TSharedPtr<FFormulaExpression> Expression;

public:
	explicit FInvokeExpression(const TSharedRef<FJsonObject>& ExpressionObj);

	virtual EFormulaExpressionType GetType() const override  { return EFormulaExpressionType::InvokeExpression; }
	
	virtual bool IsNullPropagationEnabled() const override;
};