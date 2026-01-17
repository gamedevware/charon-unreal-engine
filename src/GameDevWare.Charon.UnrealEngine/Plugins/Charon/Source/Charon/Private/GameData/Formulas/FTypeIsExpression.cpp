#include "GameData/Formulas/FTypeIsExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaConstants.h"
#include "GameData/Formulas/IFormulaTypeDescription.h"

FTypeIsExpression::FTypeIsExpression(const TSharedRef<FJsonObject>& ExpressionObj)
{
	Type = FExpressionBuildHelper::GetTypeRef(ExpressionObj, FormulaConstants::TYPE_ATTRIBUTE);
	Expression = FExpressionBuildHelper::GetExpression(ExpressionObj, FormulaConstants::EXPRESSION_ATTRIBUTE);
}

FFormulaInvokeResult FTypeIsExpression::Invoke(const FFormulaExecutionContext& Context)
{
	const auto Result = this->Expression->Invoke(Context);
	if (Result.IsType<FFormulaInvokeError>())
	{
		return Result; // propagate error
	}
	const auto ToType = Context.TypeResolver->GetTypeDescription(this->Type);
	if (!ToType)
	{
		return MakeErrorResult(FFormulaInvokeError::UnableToResolveType(this->Type->GetFullName(/* include generics */ true)));
	}

	FFormulaValue Target = Result.Get<FFormulaValue>();
	bool bIsAssignable = ToType->IsAssignableFrom(Context.TypeResolver->GetTypeDescription(Target.GetType()));
}
