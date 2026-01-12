#include "GameData/Formulas/FConstantExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaConstants.h"

FConstantExpression::FConstantExpression(const TSharedRef<FJsonObject>& ExpressionObj)
{
	Type = FExpressionBuildHelper::GetTypeRef(ExpressionObj, FormulaConstants::TYPE_ATTRIBUTE);

	Value.Set<nullptr_t>(nullptr);
	TSharedPtr<FJsonValue> ValueField = ExpressionObj->TryGetField(FormulaConstants::VALUE_ATTRIBUTE);
	if (ValueField.IsValid())
	{
		bool bValue;
		if (ValueField->TryGetBool(bValue))
		{
			Value.Set<bool>(bValue);
		}
		double DoubleValue;
		if (ValueField->TryGetNumber(DoubleValue))
		{
			Value.Set<double>(DoubleValue);
		}
		FString StringValue;
		if (ValueField->TryGetString(StringValue))
		{
			Value.Set<FString>(StringValue);
		}
	}
}
