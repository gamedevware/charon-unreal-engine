#pragma once

#include "FFormulaExpression.h"
#include "FFormulaTypeReference.h"

class CHARON_API FTypeIsExpression final : public FFormulaExpression
{
public:
	TSharedPtr<FFormulaTypeReference> Type;
	TSharedPtr<FFormulaExpression> Expression;
	
	explicit FTypeIsExpression(const TSharedRef<FJsonObject>& ExpressionObj);

	virtual EFormulaExpressionType GetType() const override  { return EFormulaExpressionType::TypeIsExpression; }

	virtual FFormulaInvokeResult Invoke(const FFormulaExecutionContext& Context) override; 
};
