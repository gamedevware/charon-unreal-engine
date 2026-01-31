// Copyright GameDevWare, Denis Zykov 2025

#include "GameData/Formulas/Expressions/FBinaryExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaFunction.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/IFormulaType.h"

DEFINE_LOG_CATEGORY(LogBinaryExpression);

EBinaryOperationType MapToBinaryOperationType(const FString& ExpressionType)
{
	EBinaryOperationType BinaryOperationType = EBinaryOperationType::Add;

	if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_DIVIDE_CHECKED)
	{
		BinaryOperationType = EBinaryOperationType::DivideChecked;
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_DIVIDE)
	{
		BinaryOperationType = EBinaryOperationType::Divide;
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_MULTIPLY_CHECKED)
	{
		BinaryOperationType = EBinaryOperationType::MultiplyChecked;
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_MULTIPLY)
	{
		BinaryOperationType = EBinaryOperationType::Multiply;
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_MODULO)
	{
		BinaryOperationType = EBinaryOperationType::Modulo;
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_ADD_CHECKED)
	{
		BinaryOperationType = EBinaryOperationType::AddChecked;
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_ADD)
	{
		BinaryOperationType = EBinaryOperationType::Add;
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_POWER)
	{
		BinaryOperationType = EBinaryOperationType::Power;
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_SUBTRACT_CHECKED)
	{
		BinaryOperationType = EBinaryOperationType::SubtractChecked;
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_SUBTRACT)
	{
		BinaryOperationType = EBinaryOperationType::Subtract;
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_LEFT_SHIFT)
	{
		BinaryOperationType = EBinaryOperationType::LeftShift;
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_RIGHT_SHIFT)
	{
		BinaryOperationType = EBinaryOperationType::RightShift;
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_GREATER_THAN)
	{
		BinaryOperationType = EBinaryOperationType::GreaterThan;
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_GREATER_THAN_OR_EQUAL)
	{
		BinaryOperationType = EBinaryOperationType::GreaterThanOrEqual;
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_LESS_THAN)
	{
		BinaryOperationType = EBinaryOperationType::LessThan;
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_LESS_THAN_OR_EQUAL)
	{
		BinaryOperationType = EBinaryOperationType::LessThanOrEqual;
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_EQUAL)
	{
		BinaryOperationType = EBinaryOperationType::Equal;
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_NOT_EQUAL)
	{
		BinaryOperationType = EBinaryOperationType::NotEqual;
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_AND)
	{
		BinaryOperationType = EBinaryOperationType::And;
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_OR)
	{
		BinaryOperationType = EBinaryOperationType::Or;
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_EXCLUSIVE_OR)
	{
		BinaryOperationType = EBinaryOperationType::ExclusiveOr;
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_AND_ALSO)
	{
		BinaryOperationType = EBinaryOperationType::AndAlso;
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_OR_ELSE)
	{
		BinaryOperationType = EBinaryOperationType::OrElse;
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_COALESCE)
	{
		BinaryOperationType = EBinaryOperationType::Coalesce;
	}
	else
	{
		UE_LOG(LogBinaryExpression, Warning, TEXT("Unknown binary operation type %s."), *ExpressionType);
	}

	return BinaryOperationType;
}

FString GetBinaryOperationName(EBinaryOperationType BinaryOperation)
{
	switch (BinaryOperation)
	{
	case EBinaryOperationType::And: return TEXT("&");
	case EBinaryOperationType::Or: return TEXT("|");
	case EBinaryOperationType::ExclusiveOr: return TEXT("^");
	case EBinaryOperationType::Multiply: return TEXT("*");
	case EBinaryOperationType::MultiplyChecked: return TEXT("*");
	case EBinaryOperationType::Divide: return TEXT("/");
	case EBinaryOperationType::DivideChecked: return TEXT("/");
	case EBinaryOperationType::Power: return TEXT("**");
	case EBinaryOperationType::Modulo: return TEXT("%");
	case EBinaryOperationType::Add: return TEXT("+");
	case EBinaryOperationType::AddChecked: return TEXT("+");
	case EBinaryOperationType::Subtract: return TEXT("-");
	case EBinaryOperationType::SubtractChecked: return TEXT("-");
	case EBinaryOperationType::LeftShift: return TEXT("<<");
	case EBinaryOperationType::RightShift: return TEXT(">>");
	case EBinaryOperationType::GreaterThan: return TEXT(">");
	case EBinaryOperationType::GreaterThanOrEqual: return TEXT(">=");
	case EBinaryOperationType::LessThan: return TEXT("<");
	case EBinaryOperationType::LessThanOrEqual: return TEXT("<=");
	case EBinaryOperationType::Equal: return TEXT("==");
	case EBinaryOperationType::NotEqual: return TEXT("!=");
	case EBinaryOperationType::AndAlso: return TEXT("&&");
	case EBinaryOperationType::OrElse: return TEXT("||");
	case EBinaryOperationType::Coalesce: return TEXT("??");
	default: return TEXT("Unknown");
	}
}

FBinaryExpression::FBinaryExpression(const TSharedRef<FJsonObject>& ExpressionObj) :
	Left(FExpressionBuildHelper::GetExpression(ExpressionObj, FFormulaNotation::LEFT_ATTRIBUTE)),
	Right(FExpressionBuildHelper::GetExpression(ExpressionObj, FFormulaNotation::RIGHT_ATTRIBUTE)),
	BinaryOperationType(MapToBinaryOperationType(
		FExpressionBuildHelper::GetString(ExpressionObj, FFormulaNotation::EXPRESSION_TYPE_ATTRIBUTE)))
{
}

FBinaryExpression::FBinaryExpression(const TSharedPtr<FFormulaExpression>& Left,
	const TSharedPtr<FFormulaExpression>& Right, const EBinaryOperationType BinaryOperationType) :
	Left(Left), Right(Right), BinaryOperationType(BinaryOperationType)
{
}

FFormulaExecutionResult FBinaryExpression::Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const
{
	if (!this->IsValid())
	{
		return FFormulaExecutionError::ExpressionIsInvalid();
	}

	// ReSharper disable once CppIncompleteSwitchStatement, CppDefaultCaseNotHandledInSwitchStatement
	switch (this->BinaryOperationType)
	{
	case EBinaryOperationType::Coalesce:
		return this->ExecuteCoalesce(Context);
	case EBinaryOperationType::AndAlso:
	case EBinaryOperationType::OrElse:
		return this->ExecuteJunction(Context);
	}

	const auto LeftResult = this->Left->Execute(Context, nullptr);
	if (LeftResult.HasError())
	{
		return LeftResult; // propagate error
	}
	const auto LeftOperand = LeftResult.GetValue();

	const auto RightResult = this->Right->Execute(Context, nullptr);
	if (RightResult.HasError())
	{
		return RightResult; // propagate error
	}
	const auto RightOperand = RightResult.GetValue();

	if (this->BinaryOperationType == EBinaryOperationType::Power)
	{
		return this->ExecutePower(LeftOperand, RightOperand);
	}
	else if (LeftOperand->IsNull() || RightOperand->IsNull())
	{
		if (LeftOperand->GetTypeCode() == EFormulaValueType::Boolean || RightOperand->GetTypeCode() == EFormulaValueType::Boolean)
		{
			return ExecuteNullLiftedBoolean(LeftOperand, RightOperand);
		}
		else
		{
			return ExecuteNullLifted(LeftOperand, RightOperand);
		}
	}

	// TODO promote operands in case of FString + any
	
	return LeftOperand->VisitValue([this, &LeftOperand, &RightOperand, &Context]<typename LeftValueType>(FProperty& LeftProperty, const LeftValueType& LeftValue) -> FFormulaExecutionResult
	{
		return RightOperand->VisitValue([this, &LeftProperty, &LeftValue, &LeftOperand, &RightOperand, &Context]<typename RightValueType>(const FProperty& RightProperty, const RightValueType& RightValue) -> FFormulaExecutionResult
		{
			using LeftT = std::decay_t<LeftValueType>;
			using RightT = std::decay_t<RightValueType>;
			constexpr bool bIsOneIsPointer = std::is_pointer_v<LeftT> || std::is_pointer_v<RightT>;
			constexpr bool bNotMixedBool = !(std::is_same_v<LeftT, bool> || std::is_same_v<RightT, bool>);
			constexpr bool bNotBoolOrBothBool = bNotMixedBool || (std::is_same_v<LeftT, bool> && std::is_same_v<RightT, bool>);
			constexpr bool bIsBothIntegers = std::is_integral_v<LeftT> && std::is_integral_v<RightT>; 
			constexpr bool bNotMixedSigned = !bIsBothIntegers || std::is_signed_v<LeftT> == std::is_signed_v<RightT>;
			
			if constexpr (!bIsOneIsPointer)
			{
				// ReSharper disable once CppIncompleteSwitchStatement, CppDefaultCaseNotHandledInSwitchStatement
				switch (this->BinaryOperationType)
				{
				case EBinaryOperationType::And:
					if constexpr (requires { LeftValue & RightValue; } && bNotBoolOrBothBool)
					{
						return LeftValue & RightValue;
					}
					break;
				case EBinaryOperationType::Or:
					if constexpr (requires { LeftValue | RightValue; } && bNotBoolOrBothBool)
					{
						return LeftValue | RightValue;
					}
					break;
				case EBinaryOperationType::ExclusiveOr:
					if constexpr (requires { LeftValue ^ RightValue; } && bNotBoolOrBothBool)
					{
						return LeftValue ^ RightValue;
					}
					break;
				case EBinaryOperationType::Multiply:
				case EBinaryOperationType::MultiplyChecked:
					if constexpr (requires { LeftValue * RightValue; } && bNotMixedBool)
					{
						return LeftValue * RightValue;
					}
					break;
				case EBinaryOperationType::Divide:
				case EBinaryOperationType::DivideChecked:
					if constexpr (requires { LeftValue / RightValue; } && bNotMixedBool)
					{
						return LeftValue / RightValue;
					}
					break;
				case EBinaryOperationType::Modulo:
					if constexpr (requires { LeftValue % RightValue; } && bNotMixedBool)
					{
						return LeftValue % RightValue;
					}
					break;
				case EBinaryOperationType::Add:
				case EBinaryOperationType::AddChecked:
					if constexpr (requires { LeftValue + RightValue; } && bNotMixedBool)
					{
						return LeftValue + RightValue;
					}
					break;
				case EBinaryOperationType::Subtract:
				case EBinaryOperationType::SubtractChecked:
					if constexpr (requires { LeftValue - RightValue; } && bNotMixedBool)
					{
						return LeftValue - RightValue;
					}
					break;
				case EBinaryOperationType::LeftShift:
					if constexpr (requires { LeftValue << RightValue; } && bNotMixedBool)
					{
						return LeftValue << RightValue;
					}
					break;
				case EBinaryOperationType::RightShift:
					if constexpr (requires { LeftValue >> RightValue; } && bNotMixedBool)
					{
						return LeftValue >> RightValue;
					}
					break;
				case EBinaryOperationType::GreaterThan:
					if constexpr (requires { LeftValue > RightValue; } && bNotMixedBool && bNotMixedSigned)
					{
						return LeftValue > RightValue;
					}
					break;
				case EBinaryOperationType::GreaterThanOrEqual:
					if constexpr (requires { LeftValue >= RightValue; } && bNotMixedBool && bNotMixedSigned)
					{
						return LeftValue >= RightValue;
					}
					break;
				case EBinaryOperationType::LessThan:
					if constexpr (requires { LeftValue < RightValue; } && bNotMixedBool && bNotMixedSigned)
					{
						return LeftValue < RightValue;
					}
					break;
				case EBinaryOperationType::LessThanOrEqual:
					if constexpr (requires { LeftValue <= RightValue; } && bNotMixedBool && bNotMixedSigned)
					{
						return LeftValue <= RightValue;
					}
					break;
				case EBinaryOperationType::Equal:
					if constexpr (requires { LeftValue == RightValue; } && bNotBoolOrBothBool)
					{
						return LeftValue == RightValue;
					}
					break;
				case EBinaryOperationType::NotEqual:
					if constexpr (requires { LeftValue != RightValue; } && bNotBoolOrBothBool)
					{
						return LeftValue != RightValue;
					}
					break;
				}
			}

			// fallback to custom operations
			const auto LeftOperandType = Context.TypeResolver->GetType(LeftOperand);
			const auto RightOperandType = Context.TypeResolver->GetType(RightOperand);
			const FFormulaFunction* LeftBinaryOperation = nullptr;
			const FFormulaFunction* RightBinaryOperation = nullptr;
			if (LeftOperandType->TryGetBinaryOperation(this->BinaryOperationType, LeftBinaryOperation) ||
				RightOperandType->TryGetBinaryOperation(this->BinaryOperationType, RightBinaryOperation))
			{
				FFormulaInvokeArguments CallArguments {
					FFormulaInvokeArguments::InvokeArgument(TEXT("0"), LeftOperand, nullptr, FFormulaInvokeArguments::GetArgumentFlags(Left, LeftOperand, Context)),
					FFormulaInvokeArguments::InvokeArgument(TEXT("1"), RightOperand, nullptr, FFormulaInvokeArguments::GetArgumentFlags(Right, RightOperand, Context))
				};
				TSharedPtr<FFormulaValue> ResultValue;
				if (LeftBinaryOperation && LeftBinaryOperation->TryInvoke(FFormulaValue::Null(), CallArguments, nullptr, nullptr, ResultValue ) ||
					RightBinaryOperation && RightBinaryOperation->TryInvoke(FFormulaValue::Null(), CallArguments, nullptr, nullptr, ResultValue))
				{
					return ResultValue;
				}
			}

			// failed to perform binary operation
			return FFormulaExecutionError::MissingBinaryOperation(
				LeftProperty.GetCPPType(),
				RightProperty.GetCPPType(),
				GetBinaryOperationName(this->BinaryOperationType));
		});
	});
}

bool FBinaryExpression::IsValid() const
{
	return this->Left.IsValid() && this->Right.IsValid();
}

void FBinaryExpression::DebugPrintTo(FString& OutValue) const
{
	if (this->Left.IsValid())
	{
		if (this->Left->GetType() != EFormulaExpressionType::ConstantExpression)
		{
			OutValue.Append(TEXT("("));
		}

		this->Left->DebugPrintTo(OutValue);
		if (this->Left->GetType() != EFormulaExpressionType::ConstantExpression)
		{
			OutValue.Append(TEXT(")"));
		}
	}
	else
	{
		OutValue.Append(TEXT("#NULL#"));
	}

	OutValue.Append(TEXT(" "));
	switch (this->BinaryOperationType)
	{
		case EBinaryOperationType::And: OutValue.Append(TEXT("&")); break;
		case EBinaryOperationType::Or: OutValue.Append(TEXT("|")); break;
		case EBinaryOperationType::ExclusiveOr: OutValue.Append(TEXT("^")); break;
		case EBinaryOperationType::Multiply: OutValue.Append(TEXT("*")); break;
		case EBinaryOperationType::MultiplyChecked: OutValue.Append(TEXT("*")); break;
		case EBinaryOperationType::Divide: 
		case EBinaryOperationType::DivideChecked: OutValue.Append("/"); break;
		case EBinaryOperationType::Power: OutValue.Append((TEXT("**"))); break;
		case EBinaryOperationType::Modulo: OutValue.Append(TEXT("%")); break;
		case EBinaryOperationType::Add:
		case EBinaryOperationType::AddChecked: OutValue.Append(TEXT("+")); break;
		case EBinaryOperationType::Subtract: 
		case EBinaryOperationType::SubtractChecked: OutValue.Append(TEXT("-")); break;
		case EBinaryOperationType::LeftShift: OutValue.Append(TEXT("<<")); break;
		case EBinaryOperationType::RightShift: OutValue.Append(TEXT(">>")); break;
		case EBinaryOperationType::GreaterThan: OutValue.Append(TEXT(">")); break;
		case EBinaryOperationType::GreaterThanOrEqual: OutValue.Append(TEXT(">=")); break;
		case EBinaryOperationType::LessThan: OutValue.Append(TEXT("<")); break;
		case EBinaryOperationType::LessThanOrEqual: OutValue.Append(TEXT("<=")); break;
		case EBinaryOperationType::Equal: OutValue.Append(TEXT("==")); break;
		case EBinaryOperationType::NotEqual: OutValue.Append(TEXT("!=")); break;
		case EBinaryOperationType::AndAlso: OutValue.Append(TEXT("&&")); break;
		case EBinaryOperationType::OrElse: OutValue.Append(TEXT("||")); break;
		case EBinaryOperationType::Coalesce: OutValue.Append(TEXT("??")); break;
		default: OutValue.AppendInt(static_cast<int32>(this->BinaryOperationType)); break;
	}

	OutValue.Append(" ");

	if (this->Right.IsValid())
	{
		if (this->Right->GetType() != EFormulaExpressionType::ConstantExpression)
		{
			OutValue.Append(TEXT("("));
		}

		this->Right->DebugPrintTo(OutValue);
		if (this->Right->GetType() != EFormulaExpressionType::ConstantExpression)
		{
			OutValue.Append(TEXT(")"));
		}
	}
	else
	{
		OutValue.Append(TEXT("#NULL#"));
	}
}

FFormulaExecutionResult FBinaryExpression::ExecuteCoalesce(const FFormulaExecutionContext& Context) const
{
	const auto LeftResult = this->Left->Execute(Context, nullptr);
	if (LeftResult.HasError())
	{
		return LeftResult; // propagate error
	}
	const auto LeftOperand = LeftResult.GetValue();
	if (!LeftOperand->IsNull())
	{
		return LeftResult;
	}
	
	const auto RightResult = this->Right->Execute(Context, nullptr);
	return RightResult;
}

FFormulaExecutionResult FBinaryExpression::ExecuteJunction(const FFormulaExecutionContext& Context) const
{
	const auto LeftResult = this->Left->Execute(Context, UDotNetBoolean::GetLiteralProperty());
	if (LeftResult.HasError())
	{
		return LeftResult; // propagate error
	}
	const auto LeftOperand = LeftResult.GetValue();
	bool LeftBool;
	if (LeftOperand->GetTypeCode() != EFormulaValueType::Boolean ||
			!LeftOperand->TryGetBoolean(LeftBool))
	{
		return FFormulaExecutionError::MissingBinaryOperation(
			LeftOperand->GetCPPType(),
			TEXT("bool"),
			GetBinaryOperationName(this->BinaryOperationType));
	}

	if (this->BinaryOperationType == EBinaryOperationType::AndAlso && !LeftBool)
	{
		return false;
	}
	else if (this->BinaryOperationType == EBinaryOperationType::OrElse && LeftBool)
	{
		return true;
	}

	const auto RightResult = this->Right->Execute(Context, UDotNetBoolean::GetLiteralProperty());
	if (RightResult.HasError())
	{
		return RightResult; // propagate error
	}
	const auto RightOperand = RightResult.GetValue();
	bool RightBool;
	if (RightOperand->GetTypeCode() != EFormulaValueType::Boolean ||
			!RightOperand->TryGetBoolean(RightBool))
	{
		return FFormulaExecutionError::MissingBinaryOperation(
			LeftOperand->GetCPPType(),
			RightOperand->GetCPPType(),
			GetBinaryOperationName(this->BinaryOperationType));
	}

	if (this->BinaryOperationType == EBinaryOperationType::AndAlso)
	{
		return LeftBool && RightBool;
	}
	else
	{
		return LeftBool || RightBool;
	}
}

FFormulaExecutionResult FBinaryExpression::ExecutePower(const TSharedPtr<FFormulaValue>& LeftOperand, const TSharedPtr<FFormulaValue>& RightOperand) const
{
	if (LeftOperand->IsNull() || RightOperand->IsNull())
	{
		return FFormulaValue::Null();
	}

	double LeftValue = 0;
	double RightValue = 0;
	if (!LeftOperand->TryCopyCompleteValue(UDotNetDouble::GetLiteralProperty(), &LeftValue) ||
		!RightOperand->TryCopyCompleteValue(UDotNetDouble::GetLiteralProperty(), &RightValue))
	{
		return FFormulaExecutionError::MissingBinaryOperation(
			LeftOperand->GetCPPType(),
			RightOperand->GetCPPType(),
			GetBinaryOperationName(this->BinaryOperationType));
	}
	
	const double Result = FMath::Pow(LeftValue, RightValue);
	return MakeShared<FFormulaValue>(Result);
}

FFormulaExecutionResult FBinaryExpression::ExecuteNullLiftedBoolean(const TSharedPtr<FFormulaValue>& LeftOperand, const TSharedPtr<FFormulaValue>& RightOperand) const
{
	bool LeftBool = false;
	bool RightBool = false;
	switch (this->BinaryOperationType)
	{
	case EBinaryOperationType::And:
		if (LeftOperand->IsNull() && RightOperand->IsNull())
		{
			return FFormulaValue::Null();
		}
		else if (RightOperand->IsNull())
		{
			LeftOperand->TryGetBoolean(LeftBool);
			return LeftBool ? FFormulaValue::Null() : FFormulaValue::FalseBool();
		}
		else
		{
			RightOperand->TryGetBoolean(RightBool);
			return RightBool ? FFormulaValue::Null() : FFormulaValue::FalseBool();
		}
	case EBinaryOperationType::Or:
		if (LeftOperand->IsNull() && RightOperand->IsNull())
		{
			return FFormulaValue::Null();
		}
		else if (RightOperand->IsNull())
		{
			LeftOperand->TryGetBoolean(LeftBool);
			return LeftBool ? FFormulaValue::TrueBool() : FFormulaValue::Null();
		}
		else
		{
			RightOperand->TryGetBoolean(RightBool);
			return RightBool ? FFormulaValue::TrueBool() : FFormulaValue::Null();
		}
	default:
		return this->ExecuteNullLifted(LeftOperand, RightOperand);
	}
}

FFormulaExecutionResult FBinaryExpression::ExecuteNullLifted(const TSharedPtr<FFormulaValue>& LeftOperand,	const TSharedPtr<FFormulaValue>& RightOperand) const
{
	switch (this->BinaryOperationType)
	{
	case EBinaryOperationType::And:
	case EBinaryOperationType::Or:
	case EBinaryOperationType::ExclusiveOr:
	case EBinaryOperationType::Multiply:
	case EBinaryOperationType::MultiplyChecked:
	case EBinaryOperationType::Divide:
	case EBinaryOperationType::DivideChecked:
	case EBinaryOperationType::Power:
	case EBinaryOperationType::Modulo:
	case EBinaryOperationType::Add:
	case EBinaryOperationType::AddChecked:
	case EBinaryOperationType::Subtract:
	case EBinaryOperationType::SubtractChecked:
	case EBinaryOperationType::LeftShift:
	case EBinaryOperationType::RightShift:
		return FFormulaValue::Null();
	case EBinaryOperationType::GreaterThan:
	case EBinaryOperationType::GreaterThanOrEqual:
	case EBinaryOperationType::LessThan:
	case EBinaryOperationType::LessThanOrEqual:
		return FFormulaValue::FalseBool();
	case EBinaryOperationType::Equal:
		return LeftOperand->IsNull() == RightOperand->IsNull();
	case EBinaryOperationType::NotEqual:
		return LeftOperand->IsNull() != RightOperand->IsNull();
	default:
		// failed to perform binary operation
		return FFormulaExecutionError::MissingBinaryOperation(
			LeftOperand->GetCPPType(),
			RightOperand->GetCPPType(),
			GetBinaryOperationName(this->BinaryOperationType));
	}
}
