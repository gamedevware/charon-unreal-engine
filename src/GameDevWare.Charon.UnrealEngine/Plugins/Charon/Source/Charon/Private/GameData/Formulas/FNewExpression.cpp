#include "GameData/Formulas/FNewExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaConstants.h"

FNewExpression::FNewExpression(const TSharedRef<FJsonObject>& ExpressionObj)
{
	Type = FExpressionBuildHelper::GetTypeRef(ExpressionObj, FormulaConstants::TYPE_ATTRIBUTE);
	Arguments = FExpressionBuildHelper::GetArguments(ExpressionObj, FormulaConstants::ARGUMENTS_ATTRIBUTE);
}
