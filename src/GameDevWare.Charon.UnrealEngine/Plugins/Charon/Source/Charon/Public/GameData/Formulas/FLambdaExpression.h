#pragma once

#include "FFormulaExpression.h"

class CHARON_API FLambdaExpression : public FFormulaExpression
{
private:
	TSharedPtr<FFormulaExpression> Body;
	TArray<TPair<FString, TSharedPtr<class FFormulaTypeReference>>> Arguments;

public:
	explicit FLambdaExpression(const TSharedRef<FJsonObject>& ExpressionObj);

	virtual EFormulaExpressionType GetType() const override  { return EFormulaExpressionType::LambdaExpression; }
};
