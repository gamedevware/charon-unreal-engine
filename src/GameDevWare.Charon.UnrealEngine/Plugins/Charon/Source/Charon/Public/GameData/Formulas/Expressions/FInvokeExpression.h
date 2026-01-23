#pragma once

#include "FFormulaExpression.h"
#include "../FFormulaFunction.h"

class CHARON_API FInvokeExpression : public FFormulaExpression
{
public:
	const TMap<FString, TSharedPtr<FFormulaExpression>> Arguments;
	const TSharedPtr<FFormulaExpression> Expression;
	
	explicit FInvokeExpression(const TSharedRef<FJsonObject>& ExpressionObj);

	virtual bool IsNullPropagationEnabled() const override;

	virtual FFormulaInvokeResult Execute(const FFormulaExecutionContext& Context) const override;

	inline static EFormulaExpressionType Type = EFormulaExpressionType::InvokeExpression;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
private:
	FFormulaInvokeResult ExecuteExpression(const TSharedRef<FFormulaExpression>& TargetExpression, const FFormulaExecutionContext& Context) const;
	FFormulaInvokeResult FindAndInvokeFunction(const TSharedRef<FFormulaValue>& Target, const FString& FunctionName, const TArray<UField*>& TypeArguments, const FFormulaExecutionContext& Context) const;
	FFormulaInvokeResult InvokeFunction(const TSharedRef<FFormulaValue>& Target, const FFormulaFunction* FormulaFunction, const FString& FunctionName, const TArray<UField*>& TypeArguments, const FFormulaExecutionContext& Context) const;
	static void GetGlobalFunctionNames(TSet<FString> FunctionNames, const FFormulaExecutionContext& Context);
	
};
