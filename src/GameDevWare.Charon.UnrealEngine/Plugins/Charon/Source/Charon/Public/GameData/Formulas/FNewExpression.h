#pragma once

#include "FFormulaExpression.h"
#include "FFormulaTypeReference.h"

class CHARON_API FNewExpression : public FFormulaExpression
{
public:
	TMap<FString, TSharedPtr<FFormulaExpression>> Arguments;
	TSharedPtr<FFormulaTypeReference> Type;
	
	explicit FNewExpression(const TSharedRef<FJsonObject>& ExpressionObj);

	virtual EFormulaExpressionType GetType() const override  { return EFormulaExpressionType::NewExpression; }
};
