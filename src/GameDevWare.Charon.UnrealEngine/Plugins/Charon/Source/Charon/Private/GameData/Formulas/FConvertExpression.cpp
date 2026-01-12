#include "GameData/Formulas/FConvertExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaConstants.h"

FConvertExpression::FConvertExpression(const TSharedRef<FJsonObject>& ExpressionObj)
{
	ExpressionObj->TryGetStringField(FormulaConstants::EXPRESSION_TYPE_ATTRIBUTE, ExpressionType);
	Type = FExpressionBuildHelper::GetTypeRef(ExpressionObj, FormulaConstants::TYPE_ATTRIBUTE);
	Expression = FExpressionBuildHelper::GetExpression(ExpressionObj, FormulaConstants::EXPRESSION_ATTRIBUTE);
}
