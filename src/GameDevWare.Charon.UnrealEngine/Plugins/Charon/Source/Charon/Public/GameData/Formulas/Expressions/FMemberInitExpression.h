// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "FFormulaExpression.h"
#include "FFormulaMemberBinding.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMemberInitExpression, Log, All);

class FNewExpression;

class CHARON_API FMemberInitExpression : public FFormulaExpression
{
public:
	const TSharedPtr<FNewExpression> NewExpression;
	const TArray<TSharedPtr<FFormulaMemberBinding>> Bindings;
	
	explicit FMemberInitExpression(const TSharedRef<FJsonObject>& ExpressionObj);
	explicit FMemberInitExpression(const TSharedPtr<FNewExpression>& NewExpression, const TArray<TSharedPtr<FFormulaMemberBinding>>& Bindings);

	virtual FFormulaExecutionResult Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const override;

	inline static EFormulaExpressionType Type = EFormulaExpressionType::MemberInitExpression;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
	virtual bool IsValid() const override;
	virtual void DebugPrintTo(FString& OutValue) const override;
};
