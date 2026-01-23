#pragma once
#include "FFormulaExpression.h"

class CHARON_API FConditionExpression : public FFormulaExpression
{

public:
	const TSharedPtr<FFormulaExpression> Test;
	const TSharedPtr<FFormulaExpression> IfTrue;
	const TSharedPtr<FFormulaExpression> IfFalse;
	
	explicit FConditionExpression(const TSharedRef<FJsonObject>& ExpressionObj);

	virtual FFormulaInvokeResult Execute(const FFormulaExecutionContext& Context) const override;

	inline static EFormulaExpressionType Type = EFormulaExpressionType::ConditionExpression;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
};
