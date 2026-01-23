#pragma once

#include "EUnaryOperationType.h"
#include "FFormulaExpression.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUnaryExpression, Log, All);

class CHARON_API FUnaryExpression : public FFormulaExpression
{
public:
	const TSharedPtr<FFormulaExpression> Expression;
	const EUnaryOperationType UnaryOperationType;
	
	explicit FUnaryExpression(const TSharedPtr<FJsonObject>& ExpressionObj);

	virtual FFormulaInvokeResult Execute(const FFormulaExecutionContext& Context) const override;
	
	inline static EFormulaExpressionType Type = EFormulaExpressionType::UnaryExpression;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
};