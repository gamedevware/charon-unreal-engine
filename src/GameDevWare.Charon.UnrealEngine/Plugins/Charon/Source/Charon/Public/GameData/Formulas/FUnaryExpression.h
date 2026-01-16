#pragma once

#include "EUnaryOperationType.h"
#include "FFormulaExpression.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUnaryExpression, Log, All);

class CHARON_API FUnaryExpression : public FFormulaExpression
{
private:
	static EUnaryOperationType MapUnaryOperationType(const FString& ExpressionType);
public:
	TSharedPtr<FFormulaExpression> Expression;
	EUnaryOperationType UnaryOperationType;
	EUnaryOperationType FallbackUnaryOperationType;
	
	explicit FUnaryExpression(const TSharedPtr<FJsonObject>& ExpressionObj);

	virtual EFormulaExpressionType GetType() const override  { return EFormulaExpressionType::UnaryExpression; }
};