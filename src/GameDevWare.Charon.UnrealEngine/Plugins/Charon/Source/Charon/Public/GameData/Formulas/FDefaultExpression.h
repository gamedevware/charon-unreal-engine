#pragma once

#include "FFormulaExpression.h"
#include "FFormulaTypeReference.h"

class CHARON_API FDefaultExpression : public FFormulaExpression
{
public:
	TSharedPtr<FFormulaTypeReference> Type;
	
	explicit FDefaultExpression(const TSharedRef<FJsonObject>& ExpressionObj);

	virtual EFormulaExpressionType GetType() const override  { return EFormulaExpressionType::DefaultExpression; }

	virtual FFormulaInvokeResult Invoke(const FFormulaExecutionContext& Context) override; 
};
