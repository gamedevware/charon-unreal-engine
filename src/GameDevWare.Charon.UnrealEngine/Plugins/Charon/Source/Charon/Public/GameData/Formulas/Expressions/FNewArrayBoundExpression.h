#pragma once

#include "FFormulaExpression.h"
#include "../FFormulaTypeReference.h"

DECLARE_LOG_CATEGORY_EXTERN(LogNewArrayBoundExpression, Log, All);

class CHARON_API FNewArrayBoundExpression : public FFormulaExpression
{
public:
	const TMap<FString, TSharedPtr<FFormulaExpression>> Arguments;
	const TSharedPtr<FFormulaTypeReference> ArrayType;
	
	explicit FNewArrayBoundExpression(const TSharedRef<FJsonObject>& ExpressionObj);

	virtual FFormulaInvokeResult Execute(const FFormulaExecutionContext& Context) const override;

	inline static EFormulaExpressionType Type = EFormulaExpressionType::NewArrayBoundExpression;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
};
