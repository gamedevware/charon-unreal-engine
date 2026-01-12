#pragma once

#include "CoreMinimal.h"
#include "FFormulaExpression.h"
#include "FFormulaTypeReference.h"

class CHARON_API FTypeIsExpression final : public FFormulaExpression
{
private:
	TSharedPtr<FFormulaTypeReference> Type;
	TSharedPtr<FFormulaExpression> Expression;

public:
	explicit FTypeIsExpression(const TSharedRef<FJsonObject>& ExpressionObj);

	virtual EFormulaExpressionType GetType() const override  { return EFormulaExpressionType::TypeIsExpression; }
};
