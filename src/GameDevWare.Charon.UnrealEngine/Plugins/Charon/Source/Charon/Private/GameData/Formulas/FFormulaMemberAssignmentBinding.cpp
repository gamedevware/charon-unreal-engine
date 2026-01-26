#include "GameData/Formulas/Expressions/FFormulaMemberAssignmentBinding.h"

#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/IFormulaType.h"
#include "GameData/Formulas/Expressions/FFormulaExpression.h"

FFormulaMemberAssignmentBinding::FFormulaMemberAssignmentBinding(const TSharedRef<FJsonObject>& ExpressionObj) :
	FFormulaMemberBinding(GetExpressionRawMemberName(ExpressionObj)),
	Expression(FExpressionBuildHelper::GetExpression(ExpressionObj, FFormulaNotation::EXPRESSION_ATTRIBUTE, true))
{
}

FFormulaMemberAssignmentBinding::FFormulaMemberAssignmentBinding(const FString& RawMemberName,
                                                                 const TSharedPtr<FFormulaExpression>& Expression) :
	FFormulaMemberBinding(RawMemberName),
	Expression(Expression)
{
}

bool FFormulaMemberAssignmentBinding::IsValid() const
{
	return !RawMemberName.IsEmpty() && !MemberName.IsEmpty() && Expression.IsValid();
}

FFormulaExecutionResult FFormulaMemberAssignmentBinding::ApplyMemberChanges(const TSharedRef<FFormulaValue>& Target,
                                                                            const FFormulaProperty* Member,
                                                                            const FFormulaExecutionContext& Context) const
{
	check(Member);
	auto MemberValueResult = Expression->Execute(Context, Member->GetType());
	if (MemberValueResult.HasError())
	{
		return MemberValueResult;
	}

	const bool bSetSuccess = Member->TrySetValue(Target, MemberValueResult.GetValue());
	if (!bSetSuccess)
	{
		return FFormulaExecutionError::MemberAccessFailed(Target->GetCPPType(), this->MemberName);
	}
	return Target;
}

void FFormulaMemberAssignmentBinding::DebugPrintTo(FString& OutValue) const
{
	OutValue.Append(this->RawMemberName);
	OutValue.Append(TEXT(": "));
	
	if (this->Expression.IsValid())
	{
		this->Expression->DebugPrintTo(OutValue);
	}
	else
	{
		OutValue.Append(TEXT("#INVALID#"));
	}
	
}
