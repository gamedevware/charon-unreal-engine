
#include "GameData/Formulas/FBinaryExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaConstants.h"

DEFINE_LOG_CATEGORY(LogBinaryExpression);

EBinaryOperationType FBinaryExpression::MapToBinaryOperationType(const FString& ExpressionType)
{
    EBinaryOperationType BinaryOperationType = EBinaryOperationType::Add;

    if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_DIVIDE_CHECKED)
    {
        BinaryOperationType = EBinaryOperationType::DivideChecked;
    }
    else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_DIVIDE)
    {
        BinaryOperationType = EBinaryOperationType::Divide;
    }
    else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_MULTIPLY_CHECKED)
    {
        BinaryOperationType = EBinaryOperationType::MultiplyChecked;
    }
    else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_MULTIPLY)
    {
        BinaryOperationType = EBinaryOperationType::Multiply;
    }
    else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_MODULO)
    {
        BinaryOperationType = EBinaryOperationType::Modulo;
    }
    else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_ADD_CHECKED)
    {
        BinaryOperationType = EBinaryOperationType::AddChecked;
    }
    else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_ADD)
    {
        BinaryOperationType = EBinaryOperationType::Add;
    }
    else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_POWER)
    {
        BinaryOperationType = EBinaryOperationType::Power;
    }
    else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_SUBTRACT_CHECKED)
    {
        BinaryOperationType = EBinaryOperationType::SubtractChecked;
    }
    else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_SUBTRACT)
    {
        BinaryOperationType = EBinaryOperationType::Subtract;
    }
    else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_LEFT_SHIFT)
    {
        BinaryOperationType = EBinaryOperationType::LeftShift;
    }
    else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_RIGHT_SHIFT)
    {
        BinaryOperationType = EBinaryOperationType::RightShift;
    }
    else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_GREATER_THAN)
    {
        BinaryOperationType = EBinaryOperationType::GreaterThan;
    }
    else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_GREATER_THAN_OR_EQUAL)
    {
        BinaryOperationType = EBinaryOperationType::GreaterThanOrEqual;
    }
    else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_LESS_THAN)
    {
        BinaryOperationType = EBinaryOperationType::LessThan;
    }
    else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_LESS_THAN_OR_EQUAL)
    {
        BinaryOperationType = EBinaryOperationType::LessThanOrEqual;
    }
    else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_EQUAL)
    {
        BinaryOperationType = EBinaryOperationType::Equal;
    }
    else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_NOT_EQUAL)
    {
        BinaryOperationType = EBinaryOperationType::NotEqual;
    }
    else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_AND)
    {
        BinaryOperationType = EBinaryOperationType::And;
    }
    else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_OR)
    {
        BinaryOperationType = EBinaryOperationType::Or;
    }
    else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_EXCLUSIVE_OR)
    {
        BinaryOperationType = EBinaryOperationType::ExclusiveOr;
    }
    else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_AND_ALSO)
    {
        BinaryOperationType = EBinaryOperationType::AndAlso;
    }
    else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_OR_ELSE)
    {
        BinaryOperationType = EBinaryOperationType::OrElse;
    }
    else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_COALESCE)
    {
        BinaryOperationType = EBinaryOperationType::Coalesce;
    }
    else
    {
        UE_LOG(LogBinaryExpression, Warning, TEXT("Unknown binary operation type %s."), *ExpressionType);
    }

    return BinaryOperationType;
}

FBinaryExpression::FBinaryExpression(const TSharedRef<FJsonObject>& ExpressionObj)
{
	Left = FExpressionBuildHelper::GetExpression(ExpressionObj, FormulaConstants::LEFT_ATTRIBUTE);
	Right = FExpressionBuildHelper::GetExpression(ExpressionObj, FormulaConstants::RIGHT_ATTRIBUTE);

	FString ExpressionTypeStr = FExpressionBuildHelper::GetString(ExpressionObj, FormulaConstants::EXPRESSION_TYPE_ATTRIBUTE);
	BinaryOperationType = MapToBinaryOperationType(ExpressionTypeStr);

	// Fallback Logic
	switch (BinaryOperationType)
	{
	case EBinaryOperationType::AddChecked:      FallbackBinaryOperationType = EBinaryOperationType::Add; break;
	case EBinaryOperationType::SubtractChecked: FallbackBinaryOperationType = EBinaryOperationType::Subtract; break;
	case EBinaryOperationType::DivideChecked:   FallbackBinaryOperationType = EBinaryOperationType::Divide; break;
	case EBinaryOperationType::MultiplyChecked: FallbackBinaryOperationType = EBinaryOperationType::Multiply; break;
	default:                                   FallbackBinaryOperationType = BinaryOperationType; break;
	}
}
