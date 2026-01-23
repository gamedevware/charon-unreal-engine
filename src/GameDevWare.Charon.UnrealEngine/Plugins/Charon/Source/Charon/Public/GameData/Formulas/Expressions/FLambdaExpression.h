#pragma once

#include "FFormulaExpression.h"

class CHARON_API FLambdaExpression : public FFormulaExpression
{
public:
	const TSharedPtr<FFormulaExpression> Body;
	const TArray<TPair<FString, TSharedPtr<FFormulaTypeReference>>> Arguments;
	
	explicit FLambdaExpression(const TSharedRef<FJsonObject>& ExpressionObj);

	inline static EFormulaExpressionType Type = EFormulaExpressionType::LambdaExpression;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
};
