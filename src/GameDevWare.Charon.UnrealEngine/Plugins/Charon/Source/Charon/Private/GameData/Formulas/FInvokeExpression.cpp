#include "GameData/Formulas/FInvokeExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaConstants.h"

FInvokeExpression::FInvokeExpression(const TSharedRef<FJsonObject>& ExpressionObj)
{
	Arguments = FExpressionBuildHelper::GetArguments(ExpressionObj, FormulaConstants::ARGUMENTS_ATTRIBUTE);
	Expression = FExpressionBuildHelper::GetExpression(ExpressionObj, FormulaConstants::EXPRESSION_ATTRIBUTE);
}

bool FInvokeExpression::IsNullPropagationEnabled() const
{
	if (Expression.IsValid())
	{
		return Expression->IsNullPropagationEnabled();
	}
	return false;
}
