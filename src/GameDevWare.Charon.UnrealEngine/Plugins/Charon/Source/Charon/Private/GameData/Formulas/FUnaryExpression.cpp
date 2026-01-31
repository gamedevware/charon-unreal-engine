// Copyright GameDevWare, Denis Zykov 2025

#include "GameData/Formulas/Expressions/FUnaryExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/IFormulaType.h"

DEFINE_LOG_CATEGORY(LogUnaryExpression);

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

FUnaryExpression::FUnaryExpression(const TSharedPtr<FFormulaExpression>& Expression, const EUnaryOperationType UnaryOperationType):
	Expression(Expression), UnaryOperationType(UnaryOperationType)
{
}

FFormulaExecutionResult FUnaryExpression::Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const
{
	if (!this->IsValid())
	{
		return FFormulaExecutionError::ExpressionIsInvalid();
	}

	const auto Result = this->Expression->Execute(Context, nullptr);
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

	return Operand->VisitValue([this, &Operand, &Context]<typename ValueType>(FProperty& Property, const ValueType& InValue) -> FFormulaExecutionResult
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
		const auto OperandType = Context.TypeResolver->GetType(Operand);
		const FFormulaFunction* UnaryOperation;
		if (OperandType->TryGetUnaryOperation(this->UnaryOperationType, UnaryOperation))
		{
			FFormulaInvokeArguments CallArguments {
				FFormulaInvokeArguments::InvokeArgument(TEXT("0"), Operand, nullptr, FFormulaInvokeArguments::GetArgumentFlags(Expression, Operand, Context))
			};
			TSharedPtr<FFormulaValue> ResultValue;
			if (UnaryOperation && UnaryOperation->TryInvoke(
				FFormulaValue::Null(),
				CallArguments,
				nullptr,
				nullptr,
				ResultValue
			))
			{
				return ResultValue;
			}
		}

		// failed to perform binary operation
		return FFormulaExecutionError::MissingUnaryOperation(
			Property.GetCPPType(),
			GetUnaryOperationName(this->UnaryOperationType));
	});
}

bool FUnaryExpression::IsValid() const
{
	return this->Expression.IsValid();
}

void FUnaryExpression::DebugPrintTo(FString& OutValue) const
{
	bool bEnclose = this->Expression->GetType() != EFormulaExpressionType::ConstantExpression;
	switch (this->UnaryOperationType)
	{
	case EUnaryOperationType::UnaryPlus: OutValue.Append("+"); break;
	case EUnaryOperationType::Negate:
	case EUnaryOperationType::NegateChecked: OutValue.Append("-"); break;
	case EUnaryOperationType::Not: OutValue.Append("!"); break;
	case EUnaryOperationType::Complement: OutValue.Append("~"); break;
	case EUnaryOperationType::UncheckedScope:
		bEnclose = true;
		OutValue.Append("unchecked");
		break;
	case EUnaryOperationType::CheckedScope:
		bEnclose = true;
		OutValue.Append("checked");
		break;
	case EUnaryOperationType::Group:
		bEnclose = true;
		break;
	default:
		OutValue.AppendInt(static_cast<int32>(this->UnaryOperationType));
		break;
	}
	if (bEnclose)
	{
		OutValue.Append("(");
	}
	if (this->Expression.IsValid())
	{
		this->Expression->DebugPrintTo(OutValue);
	}
	else
	{
		OutValue.Append(TEXT("#INVALID#"));
	}
	if (bEnclose)
	{
		OutValue.Append(")");
	}
}
