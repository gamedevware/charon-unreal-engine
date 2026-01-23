#pragma once

#include "FFormulaExpression.h"

class CHARON_API FConvertExpression : public FFormulaExpression
{
public:
	const TSharedPtr<FFormulaTypeReference> ConversionType;
	const TSharedPtr<FFormulaExpression> Expression;
	const FString ExpressionType;
	
	explicit FConvertExpression(const TSharedRef<FJsonObject>& ExpressionObj);

	virtual FFormulaInvokeResult Execute(const FFormulaExecutionContext& Context) const override;

	inline static EFormulaExpressionType Type = EFormulaExpressionType::ConvertExpression;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
};
