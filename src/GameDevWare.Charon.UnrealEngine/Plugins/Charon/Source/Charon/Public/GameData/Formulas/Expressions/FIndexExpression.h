#pragma once

#include "FFormulaExpression.h"

class CHARON_API FIndexExpression : public FFormulaExpression
{
public:
	const TMap<FString, TSharedPtr<FFormulaExpression>> Arguments;
	const TSharedPtr<FFormulaExpression> Expression;
	const bool bUseNullPropagation;

	explicit FIndexExpression(const TSharedRef<FJsonObject>& ExpressionObj);
	
	virtual bool IsNullPropagationEnabled() const override;

	virtual FFormulaInvokeResult Execute(const FFormulaExecutionContext& Context) const override;

	inline static EFormulaExpressionType Type = EFormulaExpressionType::IndexExpression;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
};