#include "GameData/Formulas/FMemberExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaConstants.h"

FMemberExpression::FMemberExpression(const TSharedRef<FJsonObject>& ExpressionObj)
{
	// Equivalent to ExpressionBuildHelper.GetString logic
	FString FoundString;
	if (ExpressionObj->TryGetStringField(FormulaConstants::PROPERTY_OR_FIELD_NAME_ATTRIBUTE, FoundString))
	{
		RawMemberName = FoundString;
	}
	else
	{
		ExpressionObj->TryGetStringField(FormulaConstants::NAME_ATTRIBUTE, RawMemberName);
	}

	MemberName = (RawMemberName.Len() > 0 && RawMemberName[0] == '@') ? RawMemberName.RightChop(1) : RawMemberName;
	
	TypeArguments = FExpressionBuildHelper::GetTypeRefArguments(ExpressionObj, FormulaConstants::ARGUMENTS_ATTRIBUTE);
	Expression = FExpressionBuildHelper::GetExpression(ExpressionObj, FormulaConstants::EXPRESSION_ATTRIBUTE, true);
        
	bUseNullPropagation = false;
	ExpressionObj->TryGetBoolField(FormulaConstants::USE_NULL_PROPAGATION_ATTRIBUTE, bUseNullPropagation);
}

bool FMemberExpression::IsNullPropagationEnabled() const
{
	if (bUseNullPropagation)
	{
		return true;
	}
	
	if (Expression.IsValid())
	{
		return Expression->IsNullPropagationEnabled();
	}
	return false;
}
