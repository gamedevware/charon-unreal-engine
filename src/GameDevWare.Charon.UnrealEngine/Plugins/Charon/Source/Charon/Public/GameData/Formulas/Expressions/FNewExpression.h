// Copyright GameDevWare, Denis Zykov 2025

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
	explicit FNewExpression(const TSharedPtr<FFormulaTypeReference>& NewObjectType, const TMap<FString, TSharedPtr<FFormulaExpression>>& Arguments);

	virtual FFormulaExecutionResult Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const override;

	inline static EFormulaExpressionType Type = EFormulaExpressionType::NewExpression;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
	virtual bool IsValid() const override;
	virtual void DebugPrintTo(FString& OutValue) const override;
};
