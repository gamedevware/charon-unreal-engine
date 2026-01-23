#pragma once

#include "FFormulaExpression.h"
#include "../FFormulaTypeReference.h"

class CHARON_API FTypeOfExpression final : public FFormulaExpression
{
public:
	const TSharedPtr<FFormulaTypeReference> TypeRef;
	
	explicit FTypeOfExpression(const TSharedRef<FJsonObject>& ExpressionObj);

	virtual FFormulaInvokeResult Execute(const FFormulaExecutionContext& Context) const override;

	inline static EFormulaExpressionType Type = EFormulaExpressionType::TypeOfExpression;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
};
