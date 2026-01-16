#pragma once

#include "FFormulaExpression.h"

class CHARON_API FIndexExpression : public FFormulaExpression
{
public:
	TMap<FString, TSharedPtr<FFormulaExpression>> Arguments;
	TSharedPtr<FFormulaExpression> Expression;
	bool bUseNullPropagation;

	explicit FIndexExpression(const TSharedRef<FJsonObject>& ExpressionObj);

	virtual EFormulaExpressionType GetType() const override  { return EFormulaExpressionType::IndexExpression; }
	
	virtual bool IsNullPropagationEnabled() const override;
};