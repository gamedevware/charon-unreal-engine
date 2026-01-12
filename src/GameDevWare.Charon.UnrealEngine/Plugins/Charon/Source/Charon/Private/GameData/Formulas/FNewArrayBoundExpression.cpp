#include "GameData/Formulas/FNewArrayBoundExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaConstants.h"

FNewArrayBoundExpression::FNewArrayBoundExpression(const TSharedRef<FJsonObject>& ExpressionObj)
{
	Arguments = FExpressionBuildHelper::GetArguments(ExpressionObj, FormulaConstants::ARGUMENTS_ATTRIBUTE);
	Type = FExpressionBuildHelper::GetTypeRef(ExpressionObj, FormulaConstants::TYPE_ATTRIBUTE);
}
