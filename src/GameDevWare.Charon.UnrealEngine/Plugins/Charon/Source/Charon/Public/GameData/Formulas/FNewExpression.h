#pragma once

#include "FFormulaExpression.h"
#include "FFormulaTypeReference.h"

class CHARON_API FNewExpression : public FFormulaExpression
{
private:
	TMap<FString, TSharedPtr<FFormulaExpression>> Arguments;
	TSharedPtr<FFormulaTypeReference> Type;
	
public:
	explicit FNewExpression(const TSharedRef<FJsonObject>& ExpressionObj);

	virtual EFormulaExpressionType GetType() const override  { return EFormulaExpressionType::NewExpression; }
};
