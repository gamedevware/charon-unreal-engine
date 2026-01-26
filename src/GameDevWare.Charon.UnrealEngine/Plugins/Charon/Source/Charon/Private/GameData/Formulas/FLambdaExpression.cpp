// Copyright GameDevWare, Denis Zykov 2025

#include "GameData/Formulas/Expressions/FLambdaExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/FFormulaTypeReference.h"
#include "GameData/Formulas/Expressions/FMemberExpression.h"

static TArray<TPair<FString, TSharedPtr<FFormulaTypeReference>>> PrepareLambdaArguments(const TSharedRef<FJsonObject>& ExpressionObj)
{
	TArray<TPair<FString, TSharedPtr<FFormulaTypeReference>>> Arguments; 
	for (TTuple<FString, TSharedPtr<FFormulaExpression>>& ArgumentTuple : FExpressionBuildHelper::GetArguments(ExpressionObj, FFormulaNotation::ARGUMENTS_ATTRIBUTE))
	{
		const FMemberExpression* MemberExpression = CastExpression<FMemberExpression>(ArgumentTuple.Value.Get());
		if (!MemberExpression)
		{
			// TODO Log invalid expression
			continue;;
		}
		Arguments.Add(TPair<FString, TSharedPtr<FFormulaTypeReference>>(MemberExpression->MemberName, nullptr));
	}
	return Arguments;
}

FLambdaExpression::FLambdaExpression(const TSharedRef<FJsonObject>& ExpressionObj):
	Body(FExpressionBuildHelper::GetExpression(ExpressionObj, FFormulaNotation::EXPRESSION_ATTRIBUTE)),
	Arguments(PrepareLambdaArguments(ExpressionObj))
{
}

FFormulaExecutionResult FLambdaExpression::Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const
{
	return FFormulaExecutionError::UnsupportedExpression(TEXT("Lambda"));
}

bool FLambdaExpression::IsValid() const
{
	if (!this->Body.IsValid())
	{
		return false;
	}
	for (auto ArgumentPair : this->Arguments)
	{
		if (ArgumentPair.Key.IsEmpty())
		{
			return false;
		}
	}
	return true;
}

void FLambdaExpression::DebugPrintTo(FString& OutValue) const
{
	OutValue.Append("(");
	bool bFirstArgument = true;;
	for (auto ArgumentPair : this->Arguments)
	{
		if (!bFirstArgument)
		{
			OutValue.Append(", ");
		}
		bFirstArgument = false;
		
		if (ArgumentPair.Value.IsValid())
		{
			OutValue.Append(ArgumentPair.Value->GetFullName(true));
			OutValue.Append(" ");
		}
		if (ArgumentPair.Key.IsEmpty())
		{
			OutValue.Append(ArgumentPair.Key);
		}
		else
		{
			OutValue.Append(TEXT("#INVALID"));
		}
	}
	OutValue.Append(") => ");
	
	if (this->Body.IsValid())
	{
		this->Body->DebugPrintTo(OutValue);
	}
	else
	{
		OutValue.Append(TEXT("#INVALID#"));
	}
}
