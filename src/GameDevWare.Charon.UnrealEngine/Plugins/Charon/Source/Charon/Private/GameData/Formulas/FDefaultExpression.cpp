#include "GameData/Formulas/FDefaultExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaConstants.h"
#include "GameData/Formulas/IFormulaTypeDescription.h"

FDefaultExpression::FDefaultExpression(const TSharedRef<FJsonObject>& ExpressionObj)
{
	Type = FExpressionBuildHelper::GetTypeRef(ExpressionObj, FormulaConstants::TYPE_ATTRIBUTE);
}

FFormulaInvokeResult FDefaultExpression::Invoke(const FFormulaExecutionContext& Context)
{
	const auto ToType = Context.TypeResolver->GetTypeDescription(this->Type);
	if (!ToType)
	{
		return MakeErrorResult(FFormulaInvokeError::UnableToResolveType(this->Type->GetFullName(/* include generics */ true)));
	}
	return MakeValueResult(ToType->GetDefaultValue());
}
