#pragma once

#include "FFormulaExpression.h"
#include "../FFormulaTypeReference.h"

class CHARON_API FDefaultExpression : public FFormulaExpression
{
public:
	const TSharedPtr<FFormulaTypeReference> ValueType;
	
	explicit FDefaultExpression(const TSharedRef<FJsonObject>& ExpressionObj);
	
	virtual FFormulaInvokeResult Execute(const FFormulaExecutionContext& Context) const override;

	inline static EFormulaExpressionType Type = EFormulaExpressionType::DefaultExpression;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
};
