#include "GameData/Formulas/FTypeOfExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaConstants.h"

FTypeOfExpression::FTypeOfExpression(const TSharedRef<FJsonObject>& ExpressionObj)
{
	Type = FExpressionBuildHelper::GetTypeRef(ExpressionObj, FormulaConstants::TYPE_ATTRIBUTE);
}
