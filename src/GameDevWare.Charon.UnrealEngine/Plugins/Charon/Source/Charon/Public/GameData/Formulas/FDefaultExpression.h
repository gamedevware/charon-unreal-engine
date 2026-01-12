#pragma once

#include "CoreMinimal.h"
#include "FFormulaExpression.h"
#include "FFormulaTypeReference.h"

class CHARON_API FDefaultExpression : public FFormulaExpression
{
private:
	TSharedPtr<FFormulaTypeReference> Type;

public:
	explicit FDefaultExpression(const TSharedRef<FJsonObject>& ExpressionObj);

	virtual EFormulaExpressionType GetType() const override  { return EFormulaExpressionType::DefaultExpression; }
};
