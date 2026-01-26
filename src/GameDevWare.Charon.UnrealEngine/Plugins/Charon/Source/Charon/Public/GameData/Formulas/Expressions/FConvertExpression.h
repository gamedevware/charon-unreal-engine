// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "FFormulaExpression.h"

class CHARON_API FConvertExpression : public FFormulaExpression
{
public:
	const TSharedPtr<FFormulaTypeReference> ConversionType;
	const TSharedPtr<FFormulaExpression> Expression;
	const FString ExpressionType;
	
	explicit FConvertExpression(const TSharedRef<FJsonObject>& ExpressionObj);
	explicit FConvertExpression(const TSharedPtr<FFormulaExpression>& Expression, const TSharedPtr<FFormulaTypeReference>& ConversionType, const FString& ExpressionType);

	virtual FFormulaExecutionResult Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const override;

	inline static EFormulaExpressionType Type = EFormulaExpressionType::ConvertExpression;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
	virtual bool IsValid() const override;
	virtual void DebugPrintTo(FString& OutValue) const override;

private:
	static bool TryCoerceValue(const TSharedRef<FFormulaValue>& FromValue, const TSharedPtr<IFormulaType>& InToType, TSharedPtr<FFormulaValue>& OutResultValue);
	
};
