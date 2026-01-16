#pragma once

#include "Misc/TVariant.h"
#include "FFormulaExpression.h"
#include "FFormulaTypeReference.h"

using FConstantVariant = TVariant<
	bool,
	double, 
	FString,
	nullptr_t
>;

class CHARON_API FConstantExpression : public FFormulaExpression
{
public:
	TSharedPtr<FFormulaTypeReference> Type;
	FConstantVariant Value;
	
	explicit FConstantExpression(const TSharedRef<FJsonObject>& ExpressionObj);

	virtual EFormulaExpressionType GetType() const override  { return EFormulaExpressionType::ConstantExpression; }
};
