#pragma once

#include "FFormulaExpression.h"
#include "FFormulaTypeReference.h"

class CHARON_API FTypeOfExpression final : public FFormulaExpression
{
public:
	TSharedPtr<FFormulaTypeReference> Type;
	
	explicit FTypeOfExpression(const TSharedRef<FJsonObject>& ExpressionObj);

	virtual EFormulaExpressionType GetType() const override  { return EFormulaExpressionType::TypeOfExpression; }

	virtual FFormulaInvokeResult Invoke(const FFormulaExecutionContext& Context) override; 
};
