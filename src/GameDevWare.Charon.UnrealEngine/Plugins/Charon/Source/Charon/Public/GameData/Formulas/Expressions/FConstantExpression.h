// Copyright GameDevWare, Denis Zykov 2025

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
	FConstantExpression(const TSharedRef<FFormulaValue>& Value, const TSharedRef<FFormulaTypeReference>& ValueType);

	virtual FFormulaExecutionResult Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const override;

	inline static EFormulaExpressionType Type = EFormulaExpressionType::ConstantExpression;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
	virtual bool IsValid() const override;
	virtual void DebugPrintTo(FString& OutValue) const override;
};
