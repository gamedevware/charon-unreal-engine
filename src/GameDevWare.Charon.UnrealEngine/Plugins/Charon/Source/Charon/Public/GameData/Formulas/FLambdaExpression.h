#pragma once

#include "FFormulaExpression.h"

class CHARON_API FLambdaExpression : public FFormulaExpression
{
public:
	TSharedPtr<FFormulaExpression> Body;
	TArray<TPair<FString, TSharedPtr<class FFormulaTypeReference>>> Arguments;
	
	explicit FLambdaExpression(const TSharedRef<FJsonObject>& ExpressionObj);

	virtual EFormulaExpressionType GetType() const override  { return EFormulaExpressionType::LambdaExpression; }
};
