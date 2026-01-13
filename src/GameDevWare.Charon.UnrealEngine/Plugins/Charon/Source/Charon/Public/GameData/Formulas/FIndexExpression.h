#pragma once

#include "FFormulaExpression.h"

class CHARON_API FIndexExpression : public FFormulaExpression
{
private:
	TMap<FString, TSharedPtr<FFormulaExpression>> Arguments;
	TSharedPtr<FFormulaExpression> Expression;
	bool bUseNullPropagation;

public:
	explicit FIndexExpression(const TSharedRef<FJsonObject>& ExpressionObj);

	virtual EFormulaExpressionType GetType() const override  { return EFormulaExpressionType::IndexExpression; }
	
	virtual bool IsNullPropagationEnabled() const override;
};