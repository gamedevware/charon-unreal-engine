#pragma once

#include "FFormulaExpression.h"
#include "FFormulaTypeReference.h"

class CHARON_API FNewArrayBoundExpression : public FFormulaExpression
{
public:
	TMap<FString, TSharedPtr<FFormulaExpression>> Arguments;
	TSharedPtr<FFormulaTypeReference> Type;
	
	explicit FNewArrayBoundExpression(const TSharedRef<FJsonObject>& ExpressionObj);

	virtual EFormulaExpressionType GetType() const override  { return EFormulaExpressionType::NewArrayBoundExpression; }
};
