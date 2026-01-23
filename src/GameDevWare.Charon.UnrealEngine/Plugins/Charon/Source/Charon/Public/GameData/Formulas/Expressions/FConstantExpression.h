#pragma once

#include "Misc/TVariant.h"
#include "FFormulaExpression.h"
#include "../FFormulaTypeReference.h"

DECLARE_LOG_CATEGORY_EXTERN(LogConstantExpression, Log, All);

class CHARON_API FConstantExpression : public FFormulaExpression
{
public:
	const TSharedPtr<FFormulaTypeReference> ValueType;
	const TSharedRef<FFormulaValue> Value;
	
	explicit FConstantExpression(const TSharedRef<FJsonObject>& ExpressionObj);

	virtual FFormulaInvokeResult Execute(const FFormulaExecutionContext& Context) const override;

	inline static EFormulaExpressionType Type = EFormulaExpressionType::ConstantExpression;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
};
