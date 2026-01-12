#include "GameData/Formulas/FConditionExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaConstants.h"

FConditionExpression::FConditionExpression(const TSharedRef<FJsonObject>& ExpressionObj)
{
	Test = FExpressionBuildHelper::GetExpression(ExpressionObj, FormulaConstants::TEST_ATTRIBUTE);
	IfTrue = FExpressionBuildHelper::GetExpression(ExpressionObj, FormulaConstants::IF_TRUE_ATTRIBUTE);
	IfFalse = FExpressionBuildHelper::GetExpression(ExpressionObj, FormulaConstants::IF_FALSE_ATTRIBUTE);
}
