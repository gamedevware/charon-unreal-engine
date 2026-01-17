#pragma once
#include "FFormulaExpression.h"

class CHARON_API FConditionExpression : public FFormulaExpression
{

public:
	TSharedPtr<FFormulaExpression> Test;
	TSharedPtr<FFormulaExpression> IfTrue;
	TSharedPtr<FFormulaExpression> IfFalse;
	
	explicit FConditionExpression(const TSharedRef<FJsonObject>& ExpressionObj);

	virtual EFormulaExpressionType GetType() const override  { return EFormulaExpressionType::ConditionExpression; }

	virtual FFormulaInvokeResult Invoke(const FFormulaExecutionContext& Context) override;
};
