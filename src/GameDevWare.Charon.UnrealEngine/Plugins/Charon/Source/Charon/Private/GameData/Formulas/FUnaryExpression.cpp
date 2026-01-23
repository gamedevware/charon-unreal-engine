#include "GameData/Formulas/Expressions/FUnaryExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/IFormulaTypeDescription.h"

DEFINE_LOG_CATEGORY(LogUnaryExpression);

static TArray<UField*> EmptyTypeArgument;
EUnaryOperationType MapUnaryOperationType(const FString& ExpressionType)
{
	EUnaryOperationType UnaryOperationType = EUnaryOperationType::Group;

	if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_UNCHECKED_SCOPE)
	{
		UnaryOperationType = EUnaryOperationType::UncheckedScope;
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_CHECKED_SCOPE)
	{
		UnaryOperationType = EUnaryOperationType::CheckedScope;
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_GROUP)
	{
		UnaryOperationType = EUnaryOperationType::Group;
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_UNARY_PLUS)
	{
		UnaryOperationType = EUnaryOperationType::UnaryPlus;
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_NEGATE)
	{
		UnaryOperationType = EUnaryOperationType::Negate;
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_NEGATE_CHECKED)
	{
		UnaryOperationType = EUnaryOperationType::NegateChecked;
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_NOT)
	{
		UnaryOperationType = EUnaryOperationType::Not;
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_COMPLEMENT)
	{
		UnaryOperationType = EUnaryOperationType::Complement;
	}
	else
	{
		UE_LOG(LogUnaryExpression, Warning, TEXT("Unknown unary operation type %s."), *ExpressionType);
	}

	return UnaryOperationType;
}
FString GetUnaryOperationName(const EUnaryOperationType UnaryOperation)
{
	switch (UnaryOperation)
	{
	case EUnaryOperationType::UnaryPlus: return TEXT("+");
	case EUnaryOperationType::Negate: return TEXT("-");
	case EUnaryOperationType::NegateChecked: return TEXT("-");
	case EUnaryOperationType::Not: return TEXT("!");
	case EUnaryOperationType::Complement: return TEXT("~");
	case EUnaryOperationType::UncheckedScope:
	case EUnaryOperationType::CheckedScope:
	case EUnaryOperationType::Group: return TEXT("()");
		default: return TEXT("Unknown");
	}
}

FUnaryExpression::FUnaryExpression(const TSharedPtr<FJsonObject>& ExpressionObj) :
	Expression(FExpressionBuildHelper::GetExpression(ExpressionObj, FFormulaNotation::EXPRESSION_ATTRIBUTE)),
	UnaryOperationType(MapUnaryOperationType(FExpressionBuildHelper::GetString(ExpressionObj, FFormulaNotation::EXPRESSION_TYPE_ATTRIBUTE)))
{
}

FFormulaInvokeResult FUnaryExpression::Execute(const FFormulaExecutionContext& Context) const
{
	if (!this->Expression.IsValid())
	{
		return FFormulaInvokeError::ExpressionIsInvalid();
	}

	const auto Result = this->Expression->Execute(Context);
	if (Result.HasError())
	{
		return Result; // propagate error
	}

	const auto Operand = Result.GetValue();
	if (Operand->IsNull())
	{
		return Result; // unary operations on null lifted to null
	}

	switch (this->UnaryOperationType)
	{
	case EUnaryOperationType::Group:
	case EUnaryOperationType::UncheckedScope:
	case EUnaryOperationType::CheckedScope:
		return Result; // nop operation
	default:
		break;
	}

	return Operand->VisitValue([this, Context]<typename ValueType>(const FProperty* Property, const ValueType& InValue) -> FFormulaInvokeResult
	{
		using T = std::decay_t<ValueType>;
		constexpr bool bIsBool = std::is_same_v<bool, T>;
		constexpr bool bIsSigned = std::is_signed_v<T>;
		constexpr bool bisPointer = std::is_pointer_v<T>;
		
		if constexpr (!bisPointer)
		{
			// ReSharper disable once CppIncompleteSwitchStatement, CppDefaultCaseNotHandledInSwitchStatement
			switch (this->UnaryOperationType)
			{
			case EUnaryOperationType::UnaryPlus:
				if constexpr (requires { +InValue; }) return +InValue;
				break;

			case EUnaryOperationType::Negate:
			case EUnaryOperationType::NegateChecked:
				if constexpr (requires { -InValue; } && !bIsBool && bIsSigned) return -InValue;
				break;

			case EUnaryOperationType::Not:
				if constexpr (requires { !InValue; }) return !InValue;
				break;

			case EUnaryOperationType::Complement:
				if constexpr (requires { ~InValue; } && !bIsBool) return ~InValue;
				break;
			}
		}

		// fallback to custom operations
		const auto OperandType = Context.TypeResolver->GetTypeDescription(Property);
		const FFormulaFunction* UnaryOperation;
		if (OperandType && OperandType->TryGetUnaryOperation(this->UnaryOperationType, UnaryOperation))
		{
			const TMap<FString, TSharedRef<FFormulaValue>> ConversionArguments {
				{ TEXT("0"), MakeShared<FFormulaValue>(InValue) }
			};
			
			TSharedPtr<FFormulaValue> ResultValue;
			if (UnaryOperation && UnaryOperation->TryInvoke(
				FFormulaValue::Null(),
				ConversionArguments,
				nullptr,
				EmptyTypeArgument,
				ResultValue
			))
			{
				return ResultValue;
			}
		}

		// failed to perform binary operation
		return FFormulaInvokeError::MissingUnaryOperation(
			Property->GetCPPType(),
			GetUnaryOperationName(this->UnaryOperationType));
	});
}
