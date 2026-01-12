#pragma once
#include "FFormulaExpression.h"

class CHARON_API FConvertExpression : public FFormulaExpression
{
private:
	TSharedPtr<class FFormulaTypeReference> Type;
	TSharedPtr<FFormulaExpression> Expression;
	FString ExpressionType;

public:
	explicit FConvertExpression(const TSharedRef<FJsonObject>& ExpressionObj);

	virtual EFormulaExpressionType GetType() const override  { return EFormulaExpressionType::ConvertExpression; }
};
