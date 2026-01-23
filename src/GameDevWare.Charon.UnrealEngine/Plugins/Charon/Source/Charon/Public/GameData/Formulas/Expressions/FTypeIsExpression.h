#pragma once

#include "FFormulaExpression.h"
#include "../FFormulaTypeReference.h"

class CHARON_API FTypeIsExpression final : public FFormulaExpression
{
public:
	const TSharedPtr<FFormulaTypeReference> CheckType;
	const TSharedPtr<FFormulaExpression> Expression;
	
	explicit FTypeIsExpression(const TSharedRef<FJsonObject>& ExpressionObj);

	virtual FFormulaInvokeResult Execute(const FFormulaExecutionContext& Context) const override; 
	
	inline static EFormulaExpressionType Type = EFormulaExpressionType::TypeIsExpression;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
};
