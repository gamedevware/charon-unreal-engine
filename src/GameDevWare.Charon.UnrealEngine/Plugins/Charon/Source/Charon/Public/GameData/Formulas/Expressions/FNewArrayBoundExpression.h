// Copyright GameDevWare, Denis Zykov 2025

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
	explicit FNewArrayBoundExpression(const TSharedPtr<FFormulaTypeReference>& ArrayType, const TMap<FString, TSharedPtr<FFormulaExpression>>& Arguments);
	
	virtual FFormulaExecutionResult Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const override;

	inline static EFormulaExpressionType Type = EFormulaExpressionType::NewArrayBoundExpression;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
	virtual bool IsValid() const override;
	virtual void DebugPrintTo(FString& OutValue) const override;

	static bool TryCreateArray(const TSharedPtr<IFormulaType>& ElementType, FArrayProperty* ArrayProperty, TSharedPtr<FFormulaValue>&
	                           OutCreatedArray);
};
