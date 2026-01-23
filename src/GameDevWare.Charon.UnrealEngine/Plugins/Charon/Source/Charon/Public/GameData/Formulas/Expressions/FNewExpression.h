#pragma once

#include "FFormulaExpression.h"
#include "../FFormulaTypeReference.h"

DECLARE_LOG_CATEGORY_EXTERN(LogNewExpression, Log, All);

class CHARON_API FNewExpression : public FFormulaExpression
{
public:
	const TMap<FString, TSharedPtr<FFormulaExpression>> Arguments;
	const TSharedPtr<FFormulaTypeReference> NewObjectType;
	
	explicit FNewExpression(const TSharedRef<FJsonObject>& ExpressionObj);

	virtual FFormulaInvokeResult Execute(const FFormulaExecutionContext& Context) const override;

	inline static EFormulaExpressionType Type = EFormulaExpressionType::NewExpression;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
};
