#include "GameData/Formulas/FConditionExpression.h"
#include "FDotNetBoolean.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaConstants.h"

FConditionExpression::FConditionExpression(const TSharedRef<FJsonObject>& ExpressionObj)
{
	Test = FExpressionBuildHelper::GetExpression(ExpressionObj, FormulaConstants::TEST_ATTRIBUTE);
	IfTrue = FExpressionBuildHelper::GetExpression(ExpressionObj, FormulaConstants::IF_TRUE_ATTRIBUTE);
	IfFalse = FExpressionBuildHelper::GetExpression(ExpressionObj, FormulaConstants::IF_FALSE_ATTRIBUTE);
}

FFormulaInvokeResult FConditionExpression::Invoke(const FFormulaExecutionContext& Context)
{
	const auto Result = this->Test->Invoke(Context);
	if (Result.IsType<FFormulaInvokeError>())
	{
		return Result; // propagate error
	}

	const auto TestValue = Result.Get<FFormulaValue>();
	bool ConditionResult = false;
	if (TestValue.GetTypeCode() == EFormulaValueType::Boolean)
	{
		TestValue.TryCopyCompleteValue(FDotNetBoolean::GetLiteralProperty(), &ConditionResult);
		if (ConditionResult)
		{
			return this->IfTrue->Invoke(Context);
		}
		else
		{
			return this->IfFalse->Invoke(Context);
		}
	}
	
	return this->MakeErrorResult(
		FFormulaInvokeError::InvalidConditionResultType(TestValue.GetType()->GetCPPType())
	);
}
