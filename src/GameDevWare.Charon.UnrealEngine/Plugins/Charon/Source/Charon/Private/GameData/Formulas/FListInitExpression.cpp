#include "GameData/Formulas/Expressions/FListInitExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/Expressions/FFormulaElementInitBinding.h"
#include "GameData/Formulas/Expressions/FNewExpression.h"

FListInitExpression::FListInitExpression(const TSharedRef<FJsonObject>& ExpressionObj):
	NewExpression(CastExpression<FNewExpression>(FExpressionBuildHelper::GetExpression(ExpressionObj, FFormulaNotation::NEW_ATTRIBUTE))),
	Initializers(FExpressionBuildHelper::GetElementInitBindings(ExpressionObj, FFormulaNotation::INITIALIZERS_ATTRIBUTE))
{
	
}

FListInitExpression::FListInitExpression(const TSharedPtr<FNewExpression>& NewExpression,
	const TArray<TSharedPtr<FFormulaElementInitBinding>>& Initializers) : NewExpression(NewExpression), Initializers(Initializers)
	
{
}

FFormulaExecutionResult FListInitExpression::Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const
{
	if (!this->IsValid())
	{
		return FFormulaExecutionError::ExpressionIsInvalid();
	}

	const auto NewListResult = this->NewExpression->Execute(Context, ExpectedType);
	if (NewListResult.HasError())
	{
		return NewListResult; // propagate error
	}

	const auto NewListValue = NewListResult.GetValue();
	for (const auto ElementInitBinding : this->Initializers)
	{
		const auto ElementInitResult = ElementInitBinding->Apply(NewListValue, Context);
		if (ElementInitResult.HasError())
		{
			return ElementInitResult;
		}
	}
	
	FFormulaElementInitBinding::CompleteCollectionInitialization(NewListValue);

	return NewListResult;
}

bool FListInitExpression::IsValid() const
{
	if (!this->NewExpression.IsValid())
	{
		return  false;
	}
	for (auto Initializer : this->Initializers)
	{
		if (!Initializer.IsValid())
		{
			return false;
		}
	}
	return true;
}

void FListInitExpression::DebugPrintTo(FString& OutValue) const
{
	if (this->NewExpression.IsValid())
	{
		this->NewExpression->DebugPrintTo(OutValue);
	}
	else
	{
		OutValue.Append(TEXT("#INVALID#"));
	}
	OutValue.Append(TEXT(" { "));
	bool bFirstArgument = true;
	for (auto ListInitBinding : this->Initializers)
	{
		if (!bFirstArgument)
		{
			OutValue.Append(", ");
		}
		bFirstArgument = false;

		if (ListInitBinding)
		{
			if (ListInitBinding->Initializers.Num() > 1)
			{
				OutValue.Append(TEXT("{ "));
			}
			ListInitBinding->DebugPrintTo(OutValue);
			if (ListInitBinding->Initializers.Num() > 1)
			{
				OutValue.Append(TEXT(" }"));
			}
		}
		else
		{
			OutValue.Append(TEXT("#INVALID#"));
		}
	}
	OutValue.Append(TEXT(" }"));
}
