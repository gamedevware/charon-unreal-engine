// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "FFormulaExpression.h"
#include "../FFormulaTypeReference.h"

DECLARE_LOG_CATEGORY_EXTERN(FNLogNewArrayInitExpression, Log, All);

class CHARON_API FNewArrayInitExpression : public FFormulaExpression
{
public:
	const TArray<TSharedPtr<FFormulaExpression>> Initializers;
	const TSharedPtr<FFormulaTypeReference> ArrayType;
	
	explicit FNewArrayInitExpression(const TSharedRef<FJsonObject>& ExpressionObj);
	explicit FNewArrayInitExpression(const TSharedPtr<FFormulaTypeReference>& ArrayType, const TArray<TSharedPtr<FFormulaExpression>>& Initializers);
	
	virtual FFormulaExecutionResult Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const override;

	inline static EFormulaExpressionType Type = EFormulaExpressionType::NewArrayInitExpression;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
	
	virtual bool IsValid() const override;
	virtual void DebugPrintTo(FString& OutValue) const override;
};
