// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "FFormulaExpression.h"
#include "../FFormulaFunction.h"

DECLARE_LOG_CATEGORY_EXTERN(LogInvokeExpression, Log, All);

class CHARON_API FInvokeExpression : public FFormulaExpression
{
public:
	const TMap<FString, TSharedPtr<FFormulaExpression>> Arguments;
	const TSharedPtr<FFormulaExpression> Expression;
	
	explicit FInvokeExpression(const TSharedRef<FJsonObject>& ExpressionObj);
	explicit FInvokeExpression(const TSharedPtr<FFormulaExpression>& Expression, const TMap<FString, TSharedPtr<FFormulaExpression>>& Arguments);

	virtual bool IsNullPropagationEnabled() const override;

	virtual FFormulaExecutionResult Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const override;

	inline static EFormulaExpressionType Type = EFormulaExpressionType::InvokeExpression;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
	virtual bool IsValid() const override;
	virtual void DebugPrintTo(FString& OutValue) const override;
private:
	FFormulaExecutionResult ExecuteExpression(const TSharedRef<FFormulaExpression>& TargetExpression, const FFormulaExecutionContext& Context) const;
	FFormulaExecutionResult FindAndInvokeFunction(const TSharedRef<FFormulaValue>& Target, const FString& FunctionName, const TArray<UField*>* TypeArguments, const FFormulaExecutionContext& Context) const;
	FFormulaExecutionResult InvokeFunction(const TSharedRef<FFormulaValue>& Target, const FFormulaFunction* FormulaFunction, const FString& FunctionName, const TArray<UField*>* TypeArguments, const FFormulaExecutionContext& Context) const;
	static void GetGlobalFunctionNames(TSet<FString> FunctionNames, const FFormulaExecutionContext& Context);
	
};
