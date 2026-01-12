#include "GameData/Formulas/FLambdaExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaConstants.h"
#include "GameData/Formulas/FMemberExpression.h"

FLambdaExpression::FLambdaExpression(const TSharedRef<FJsonObject>& ExpressionObj)
{
	Body = FExpressionBuildHelper::GetExpression(ExpressionObj, FormulaConstants::EXPRESSION_ATTRIBUTE);
	
	auto ArgsMap = FExpressionBuildHelper::GetArguments(ExpressionObj, FormulaConstants::ARGUMENTS_ATTRIBUTE);
	for (TTuple<FString, TSharedPtr<FFormulaExpression>>& It : ArgsMap)
	{
		if (!It.Value.IsValid() || It.Value->GetType() !=  EFormulaExpressionType::MemberExpression)
		{
			// TODO Log invalid expression
			continue;;
		}
		
		TSharedPtr<FMemberExpression> MemberExpr = StaticCastSharedPtr<FMemberExpression>(It.Value);
		if (!MemberExpr.IsValid())
		{
			// TODO Log invalid expression
			continue;
		}

		Arguments.Add(TPair<FString, TSharedPtr<FFormulaTypeReference>>(MemberExpr->GetMemberName(), nullptr));
	}
}
