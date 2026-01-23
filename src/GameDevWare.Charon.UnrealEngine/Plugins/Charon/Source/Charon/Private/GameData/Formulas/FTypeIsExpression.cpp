#include "GameData/Formulas/Expressions/FTypeIsExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/IFormulaTypeDescription.h"

FTypeIsExpression::FTypeIsExpression(const TSharedRef<FJsonObject>& ExpressionObj):
	CheckType(FExpressionBuildHelper::GetTypeRef(ExpressionObj, FFormulaNotation::TYPE_ATTRIBUTE)),
	Expression(FExpressionBuildHelper::GetExpression(ExpressionObj, FFormulaNotation::EXPRESSION_ATTRIBUTE))
{}

FFormulaInvokeResult FTypeIsExpression::Execute(const FFormulaExecutionContext& Context) const
{
	if (!this->Expression.IsValid() || !this->CheckType.IsValid())
	{
		return FFormulaInvokeError::ExpressionIsInvalid();
	}
	
	const auto Result = this->Expression->Execute(Context);
	if (Result.HasError())
	{
		return Result; // propagate error
	}
	const auto ToType = Context.TypeResolver->GetTypeDescription(this->CheckType);
	if (!ToType)
	{
		return FFormulaInvokeError::UnableToResolveType(this->CheckType->GetFullName(/* include generics */ true));
	}

	const auto Target = Result.GetValue();
	const bool bIsAssignable = ToType->IsAssignableFrom(Target->GetType());
	return MakeShared<FFormulaValue>(bIsAssignable);
}
