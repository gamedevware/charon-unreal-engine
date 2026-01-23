#include "GameData/Formulas/Expressions/FLambdaExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/Expressions/FMemberExpression.h"

static TArray<TPair<FString, TSharedPtr<FFormulaTypeReference>>> PrepareLambdaArguments(const TSharedRef<FJsonObject>& ExpressionObj)
{
	TArray<TPair<FString, TSharedPtr<FFormulaTypeReference>>> Arguments; 
	for (TTuple<FString, TSharedPtr<FFormulaExpression>>& ArgumentTuple : FExpressionBuildHelper::GetArguments(ExpressionObj, FFormulaNotation::ARGUMENTS_ATTRIBUTE))
	{
		const FMemberExpression* MemberExpression = CastExpression<FMemberExpression>(ArgumentTuple.Value.Get());
		if (!MemberExpression)
		{
			// TODO Log invalid expression
			continue;;
		}
		Arguments.Add(TPair<FString, TSharedPtr<FFormulaTypeReference>>(MemberExpression->MemberName, nullptr));
	}
	return Arguments;
}

FLambdaExpression::FLambdaExpression(const TSharedRef<FJsonObject>& ExpressionObj):
	Body(FExpressionBuildHelper::GetExpression(ExpressionObj, FFormulaNotation::EXPRESSION_ATTRIBUTE)),
	Arguments(PrepareLambdaArguments(ExpressionObj))
{
}
