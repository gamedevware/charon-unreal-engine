#pragma once
#include "FFormulaExpression.h"

class CHARON_API FConditionExpression : public FFormulaExpression
{
private:
	TSharedPtr<FFormulaExpression> Test;
	TSharedPtr<FFormulaExpression> IfTrue;
	TSharedPtr<FFormulaExpression> IfFalse;

public:
	explicit FConditionExpression(const TSharedRef<FJsonObject>& ExpressionObj);

	virtual EFormulaExpressionType GetType() const override  { return EFormulaExpressionType::ConditionExpression; }
};
