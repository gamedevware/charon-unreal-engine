#include "GameData/Formulas/FIndexExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaConstants.h"

FIndexExpression::FIndexExpression(const TSharedRef<FJsonObject>& ExpressionObj)
{
	Arguments = FExpressionBuildHelper::GetArguments(ExpressionObj, FormulaConstants::ARGUMENTS_ATTRIBUTE);
	Expression = FExpressionBuildHelper::GetExpression(ExpressionObj, FormulaConstants::EXPRESSION_ATTRIBUTE);
        
	// GetValueOrDefault() logic
	bUseNullPropagation = false;
	ExpressionObj->TryGetBoolField(FormulaConstants::USE_NULL_PROPAGATION_ATTRIBUTE, bUseNullPropagation);
}

bool FIndexExpression::IsNullPropagationEnabled() const
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
