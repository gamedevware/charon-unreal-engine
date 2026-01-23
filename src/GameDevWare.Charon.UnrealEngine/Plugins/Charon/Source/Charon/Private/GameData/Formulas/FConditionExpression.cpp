#include "GameData/Formulas/Expressions/FConditionExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"

FConditionExpression::FConditionExpression(const TSharedRef<FJsonObject>& ExpressionObj) :
	Test(FExpressionBuildHelper::GetExpression(ExpressionObj, FFormulaNotation::TEST_ATTRIBUTE)),
	IfTrue(FExpressionBuildHelper::GetExpression(ExpressionObj, FFormulaNotation::IF_TRUE_ATTRIBUTE)),
	IfFalse(FExpressionBuildHelper::GetExpression(ExpressionObj, FFormulaNotation::IF_FALSE_ATTRIBUTE))
{
}

FFormulaInvokeResult FConditionExpression::Execute(const FFormulaExecutionContext& Context) const
{
	const auto Result = this->Test->Execute(Context);
	if (Result.HasError())
	{
		return Result; // propagate error
	}

	const auto TestValue = Result.GetValue();
	if (bool ConditionResult = false; TestValue->TryGetBoolean(ConditionResult))
	{
		if (ConditionResult)
		{
			return this->IfTrue->Execute(Context);
		}
		else
		{
			return this->IfFalse->Execute(Context);
		}
	}

	return FFormulaInvokeError::InvalidConditionResultType(TestValue->GetType()->GetCPPType());
}
