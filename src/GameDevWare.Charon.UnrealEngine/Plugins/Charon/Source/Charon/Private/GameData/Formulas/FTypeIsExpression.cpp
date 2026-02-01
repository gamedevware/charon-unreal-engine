// Copyright GameDevWare, Denis Zykov 2025

#include "GameData/Formulas/Expressions/FTypeIsExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/IFormulaType.h"

FTypeIsExpression::FTypeIsExpression(const TSharedRef<FJsonObject>& ExpressionObj):
	CheckType(FExpressionBuildHelper::GetTypeRef(ExpressionObj, FFormulaNotation::TYPE_ATTRIBUTE)),
	Expression(FExpressionBuildHelper::GetExpression(ExpressionObj, FFormulaNotation::EXPRESSION_ATTRIBUTE))
{}

FTypeIsExpression::FTypeIsExpression(const TSharedPtr<FFormulaExpression>& Expression,
	const TSharedPtr<FFormulaTypeReference>& CheckType) :
	CheckType(CheckType), Expression(Expression)
{
}

FFormulaExecutionResult FTypeIsExpression::Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const
{
	if (!this->IsValid())
	{
		return FFormulaExecutionError::ExpressionIsInvalid();
	}
	
	const auto Result = this->Expression->Execute(Context, nullptr);
	if (Result.HasError())
	{
		return Result; // propagate error
	}
	const auto ToType = Context.TypeResolver->FindType(this->CheckType);
	if (!ToType.IsValid())
	{
		return FFormulaExecutionError::UnableToResolveType(this->CheckType->GetFullName(/* include generics */ true));
	}

	const auto Target = Result.GetValue();
	const bool bIsAssignable = ToType->IsAssignableFrom(Target->GetType(), /*bWithoutConversion*/ true);
	return MakeShared<FFormulaValue>(bIsAssignable);
}

bool FTypeIsExpression::IsValid() const
{
	return this->CheckType.IsValid() && this->Expression.IsValid();
}

void FTypeIsExpression::DebugPrintTo(FString& OutValue) const
{
	if (this->Expression.IsValid())
	{
		this->Expression->DebugPrintTo(OutValue);
	}
	else
	{
		OutValue.Append(TEXT("#INVALID#"));
	}
		
	OutValue.Append(" is ");
	if (this->CheckType.IsValid())
	{
		OutValue.Append(this->CheckType->GetFullName(true));
	}
	else
	{
		OutValue.Append(TEXT("#INVALID#"));
	}
}
