#include "GameData/Formulas/FUnaryExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaConstants.h"

DEFINE_LOG_CATEGORY(LogUnaryExpression);

EUnaryOperationType FUnaryExpression::MapUnaryOperationType(const FString& ExpressionType)
{
	EUnaryOperationType UnaryOperationType = EUnaryOperationType::Group;

	if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_UNCHECKED_SCOPE)
	{
		UnaryOperationType = EUnaryOperationType::UncheckedScope;
	}
	else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_CHECKED_SCOPE)
	{
		UnaryOperationType = EUnaryOperationType::CheckedScope;
	}
	else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_GROUP)
	{
		UnaryOperationType = EUnaryOperationType::Group;
	}
	else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_UNARY_PLUS)
	{
		UnaryOperationType = EUnaryOperationType::UnaryPlus;
	}
	else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_NEGATE)
	{
		UnaryOperationType = EUnaryOperationType::Negate;
	}
	else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_NEGATE_CHECKED)
	{
		UnaryOperationType = EUnaryOperationType::NegateChecked;
	}
	else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_NOT)
	{
		UnaryOperationType = EUnaryOperationType::Not;
	}
	else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_COMPLEMENT)
	{
		UnaryOperationType = EUnaryOperationType::Complement;
	}
	else
	{
		UE_LOG(LogUnaryExpression, Warning, TEXT("Unknown unary operation type %s."), *ExpressionType);
	}

	return UnaryOperationType;
}

FUnaryExpression::FUnaryExpression(const TSharedPtr<FJsonObject>& ExpressionObj)
{
	Expression = FExpressionBuildHelper::GetExpression(ExpressionObj, FormulaConstants::EXPRESSION_ATTRIBUTE);

	FString ExpressionTypeStr = FExpressionBuildHelper::GetString(ExpressionObj, FormulaConstants::EXPRESSION_TYPE_ATTRIBUTE);
	UnaryOperationType = MapUnaryOperationType(ExpressionTypeStr);

	// Fallback Logic
	switch (UnaryOperationType)
	{
	case EUnaryOperationType::NegateChecked:
		FallbackUnaryOperationType = EUnaryOperationType::Negate;
		break;
	default:
		FallbackUnaryOperationType = UnaryOperationType;
		break;
	}
}
