#include "GameData/Formulas/FTypeIsExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaConstants.h"

FTypeIsExpression::FTypeIsExpression(const TSharedRef<FJsonObject>& ExpressionObj)
{
	Type = FExpressionBuildHelper::GetTypeRef(ExpressionObj, FormulaConstants::TYPE_ATTRIBUTE);
	Expression = FExpressionBuildHelper::GetExpression(ExpressionObj, FormulaConstants::EXPRESSION_ATTRIBUTE);
}
