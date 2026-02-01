// Copyright GameDevWare, Denis Zykov 2025

#include "GameData/Formulas/Expressions/FConditionExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"

FConditionExpression::FConditionExpression(const TSharedRef<FJsonObject>& ExpressionObj) :
	Test(FExpressionBuildHelper::GetExpression(ExpressionObj, FFormulaNotation::TEST_ATTRIBUTE)),
	IfTrue(FExpressionBuildHelper::GetExpression(ExpressionObj, FFormulaNotation::IF_TRUE_ATTRIBUTE)),
	IfFalse(FExpressionBuildHelper::GetExpression(ExpressionObj, FFormulaNotation::IF_FALSE_ATTRIBUTE))
{
}

FConditionExpression::FConditionExpression(const TSharedPtr<FFormulaExpression>& Test, const TSharedPtr<FFormulaExpression>& IfTrue,
	const TSharedPtr<FFormulaExpression>& IfFalse) : Test(Test), IfTrue(IfTrue), IfFalse(IfFalse)
{
	
}

FFormulaExecutionResult FConditionExpression::Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const
{
	if (!this->IsValid())
	{
		return FFormulaExecutionError::ExpressionIsInvalid();
	}
	
	const auto Result = this->Test->Execute(Context, UDotNetBoolean::GetLiteralProperty());
	if (Result.HasError())
	{
		return Result; // propagate error
	}

	const auto TestValue = Result.GetValue();
	if (bool ConditionResult; TestValue->TryGetBoolean(ConditionResult))
	{
		if (ConditionResult)
		{
			return this->IfTrue->Execute(Context, nullptr);
		}
		else
		{
			return this->IfFalse->Execute(Context, nullptr);
		}
	}

	return FFormulaExecutionError::InvalidConditionResultType(TestValue->GetCPPType());
}

bool FConditionExpression::IsValid() const
{
	return this->Test.IsValid() && this->IfTrue.IsValid() && this->IfFalse.IsValid();
}

void FConditionExpression::DebugPrintTo(FString& OutValue) const
{
	OutValue.Append("(");
	if (this->Test.IsValid())
	{
		this->Test->DebugPrintTo(OutValue);
	}
	else
	{
		OutValue.Append(TEXT("#INVALID#"));
	}
	OutValue.Append(")");
	OutValue.Append(" ? ");
	
	if (this->IfTrue.IsValid())
	{
		if (this->IfTrue->GetType() != EFormulaExpressionType::ConstantExpression)
		{
			OutValue.Append(TEXT("("));
		}

		this->IfTrue->DebugPrintTo(OutValue);
		if (this->IfTrue->GetType() != EFormulaExpressionType::ConstantExpression)
		{
			OutValue.Append(TEXT(")"));
		}
	}
	else
	{
		OutValue.Append(TEXT("#NULL#"));
	}

	OutValue.Append(" : ");
	
	if (this->IfFalse.IsValid())
	{
		if (this->IfFalse->GetType() != EFormulaExpressionType::ConstantExpression)
		{
			OutValue.Append(TEXT("("));
		}

		this->IfFalse->DebugPrintTo(OutValue);
		if (this->IfFalse->GetType() != EFormulaExpressionType::ConstantExpression)
		{
			OutValue.Append(TEXT(")"));
		}
	}
	else
	{
		OutValue.Append(TEXT("#NULL#"));
	}
}
