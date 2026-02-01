#include "GameData/Formulas/Expressions/FMemberInitExpression.h"

#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/Expressions/FNewExpression.h"

FMemberInitExpression::FMemberInitExpression(const TSharedRef<FJsonObject>& ExpressionObj):
	NewExpression(CastExpression<FNewExpression>(FExpressionBuildHelper::GetExpression(ExpressionObj, FFormulaNotation::NEW_ATTRIBUTE))),
	Bindings(FExpressionBuildHelper::GetBindings(ExpressionObj, FFormulaNotation::BINDINGS_ATTRIBUTE))
{
	
}

FMemberInitExpression::FMemberInitExpression(const TSharedPtr<FNewExpression>& NewExpression,
	const TArray<TSharedPtr<FFormulaMemberBinding>>& Bindings) : NewExpression(NewExpression), Bindings(Bindings)
	
{
}

FFormulaExecutionResult FMemberInitExpression::Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const
{
	if (!this->IsValid())
	{
		return FFormulaExecutionError::ExpressionIsInvalid();
	}

	const auto NewObjectResult = this->NewExpression->Execute(Context, ExpectedType);
	if (NewObjectResult.HasError())
	{
		return NewObjectResult; // propagate error
	}

	const auto NewObjectValue = NewObjectResult.GetValue();
	for (const auto MemberBinding : this->Bindings)
	{
		const auto MemberBindingResult = MemberBinding->Apply(NewObjectValue, Context);
		if (MemberBindingResult.HasError())
		{
			return MemberBindingResult;
		}
	}

	return NewObjectResult;
}

bool FMemberInitExpression::IsValid() const
{
	if (!this->NewExpression.IsValid())
	{
		return  false;
	}
	for (auto Binding : this->Bindings)
	{
		if (!Binding.IsValid())
		{
			return false;
		}
	}
	return true;
}

void FMemberInitExpression::DebugPrintTo(FString& OutValue) const
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
	for (auto Binding : this->Bindings)
	{
		if (!bFirstArgument)
		{
			OutValue.Append(", ");
		}
		bFirstArgument = false;

		if (Binding.IsValid())
		{
			Binding->DebugPrintTo(OutValue);
		}
		else
		{
			OutValue.Append(TEXT("#INVALID#"));
		}
	}
	OutValue.Append(TEXT(" }"));
}
