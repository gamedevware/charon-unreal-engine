// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "FFormulaExpression.h"
#include "../FFormulaTypeReference.h"

class CHARON_API FDefaultExpression : public FFormulaExpression
{
public:
	const TSharedPtr<FFormulaTypeReference> ValueType;
	
	explicit FDefaultExpression(const TSharedRef<FJsonObject>& ExpressionObj);
	explicit FDefaultExpression(const TSharedPtr<FFormulaTypeReference>& ValueType);
	
	virtual FFormulaExecutionResult Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const override;

	inline static EFormulaExpressionType Type = EFormulaExpressionType::DefaultExpression;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
	virtual bool IsValid() const override;
	virtual void DebugPrintTo(FString& OutValue) const override;
};
