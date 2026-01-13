#pragma once

#include "FFormulaExpression.h"
#include "FFormulaTypeReference.h"

class CHARON_API FTypeOfExpression final : public FFormulaExpression
{
private:
	TSharedPtr<FFormulaTypeReference> Type;

public:
	explicit FTypeOfExpression(const TSharedRef<FJsonObject>& ExpressionObj);

	virtual EFormulaExpressionType GetType() const override  { return EFormulaExpressionType::TypeOfExpression; }
};
