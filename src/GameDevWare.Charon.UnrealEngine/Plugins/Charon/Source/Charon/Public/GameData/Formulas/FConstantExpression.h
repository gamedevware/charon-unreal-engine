#pragma once

#include "Misc/TVariant.h"
#include "FFormulaExpression.h"
#include "FFormulaTypeReference.h"

DECLARE_LOG_CATEGORY_EXTERN(LogConstantExpression, Log, All);

class CHARON_API FConstantExpression : public FFormulaExpression
{
public:
	TSharedPtr<FFormulaTypeReference> Type;
	FFormulaValue Value;
	
	explicit FConstantExpression(const TSharedRef<FJsonObject>& ExpressionObj);

	virtual EFormulaExpressionType GetType() const override  { return EFormulaExpressionType::ConstantExpression; }
	
	FFormulaInvokeResult Invoke(const FFormulaExecutionContext& Context) override;
};
