// Copyright GameDevWare, Denis Zykov 2025

#include "GameData/Formulas/Expressions/FConvertExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/IFormulaType.h"
#include "GameData/Formulas/FFormulaTypeReference.h"

FConvertExpression::FConvertExpression(const TSharedRef<FJsonObject>& ExpressionObj):
	ConversionType(FExpressionBuildHelper::GetTypeRef(ExpressionObj, FFormulaNotation::TYPE_ATTRIBUTE)),
	Expression(FExpressionBuildHelper::GetExpression(ExpressionObj, FFormulaNotation::EXPRESSION_ATTRIBUTE)),
	ExpressionType(FExpressionBuildHelper::GetString(ExpressionObj, FFormulaNotation::EXPRESSION_TYPE_ATTRIBUTE))
{}

FConvertExpression::FConvertExpression(const TSharedPtr<FFormulaExpression>& Expression, const TSharedPtr<FFormulaTypeReference>& ConversionType, const FString& ExpressionType) :
	ConversionType(ConversionType),
	Expression(Expression),
	ExpressionType(ExpressionType)
{
}

bool IsCoercible(EFormulaValueType TypeCode)
{
	return  TypeCode != EFormulaValueType::DateTime && TypeCode != EFormulaValueType::Timespan &&
		TypeCode != EFormulaValueType::Struct && 
		TypeCode != EFormulaValueType::ObjectPtr &&	TypeCode != EFormulaValueType::Null; // objects and null are castable
}

FFormulaExecutionResult FConvertExpression::Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const
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

	const auto FromValue = Result.GetValue();
	const auto ToType = Context.TypeResolver->FindType(this->ConversionType);
	if (!ToType.IsValid())
	{
		return FFormulaExecutionError::UnableToResolveType(this->ConversionType->GetFullName(/* include generics */ true));
	}
	
	const auto ToTypeCode = ToType->GetTypeCode();
	if (FromValue->IsNull())
	{
		if (ToType->CanBeNull())
		{
			return Result;
		}
		else
		{
			return FFormulaExecutionError::CanConvertNullToType(ToType->GetCPPType());
		}
	}

	TSharedPtr<FFormulaValue> ResultValue;
	if (const FObjectPropertyBase* FromObjectProp = CastField<FObjectPropertyBase>(FromValue->GetType());
		FromObjectProp && ToTypeCode == EFormulaValueType::ObjectPtr)
	{
		if (FromValue->IsNull() || ToType->IsAssignableFrom(FromObjectProp->PropertyClass))
		{
			return Result; // UClass cast success
		}
		if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_TYPE_AS)
		{
			// this is not an error in `x as ClassName` expression
			return FFormulaValue::Null(); // UClass cast failure
		}
		else
		{
			return FFormulaExecutionError::InvalidCastError(FromValue->GetCPPType(), ToType->GetCPPType());
		}
	}
	else if (IsCoercible(FromValue->GetTypeCode()) && IsCoercible(ToTypeCode) && this->TryCoerceValue(FromValue, ToType, ResultValue))
	{
		return ResultValue;
	}

	const FFormulaFunction* ConversionOperation;
	if (ToType->TryGetConversionOperation(ConversionOperation) && ConversionOperation)
	{
		FFormulaInvokeArguments ConversionArguments {
			FFormulaInvokeArguments::InvokeArgument(TEXT("0"), FromValue, nullptr, FFormulaInvokeArguments::GetArgumentFlags(Expression, FromValue, Context))
		};
		if (ConversionOperation->TryInvoke(
			FFormulaValue::Null(),
			ConversionArguments,
			ToType->GetTypeClassOrStruct(),
			nullptr,
			ResultValue
		))
		{
			return ResultValue;
		}
	}

	return FFormulaExecutionError::CantConvertToType(FromValue->GetCPPType(), ToType->GetCPPType());
}

bool FConvertExpression::IsValid() const
{
	return this->ConversionType.IsValid() && this->Expression.IsValid() && !this->ExpressionType.IsEmpty();
}

void FConvertExpression::DebugPrintTo(FString& OutValue) const
{
	if (this->ExpressionType == FFormulaNotation::EXPRESSION_TYPE_TYPE_AS)
	{
		if (this->Expression.IsValid())
		{
			this->Expression->DebugPrintTo(OutValue);
		}
		else
		{
			OutValue.Append(TEXT("#INVALID#"));
		}
		
		OutValue.Append(" as ");
		if (this->ConversionType.IsValid())
		{
			OutValue.Append(this->ConversionType->GetFullName(true));
		}
		else
		{
			OutValue.Append(TEXT("#INVALID#"));
		}
	}
	else
	{
		OutValue.Append("(");
		if (this->ConversionType.IsValid())
		{
			OutValue.Append(this->ConversionType->GetFullName(true));
		}
		else
		{
			OutValue.Append(TEXT("#INVALID#"));
		}
		OutValue.Append(") ");
		if (this->Expression.IsValid())
		{
			this->Expression->DebugPrintTo(OutValue);
		}
		else
		{
			OutValue.Append(TEXT("#INVALID#"));
		}
	}
}

bool FConvertExpression::TryCoerceValue(const TSharedRef<FFormulaValue>& FromValue, const TSharedPtr<IFormulaType>& InToType,
	TSharedPtr<FFormulaValue>& OutResultValue)
{
	return FromValue->VisitValue([&InToType, &OutResultValue, &FromValue]<typename ValueType>(FProperty&, const ValueType& FromValuePtr) -> bool {

		using InT = std::decay_t<ValueType>;
		
		constexpr bool bIsInteger = std::is_integral_v<InT>;
		constexpr bool bIsFloat = std::is_floating_point_v<InT>;
		constexpr bool bIsBool = std::is_same_v<InT, bool>;
		
		auto ToTypeCode = InToType->GetTypeCode();
		if (ToTypeCode == EFormulaValueType::Enum && InToType->GetUnderlyingType().IsValid())
		{
			ToTypeCode = InToType->GetUnderlyingType()->GetTypeCode();
		}
		
		if constexpr (bIsInteger || bIsFloat || bIsBool)
		{ 
			switch (ToTypeCode)
			{
			case EFormulaValueType::Boolean:
				OutResultValue = !!FromValuePtr ? FFormulaValue::TrueBool() : FFormulaValue::FalseBool();
				return true;
			case EFormulaValueType::Int8:    
				OutResultValue = MakeShared<FFormulaValue>(static_cast<int8>(FromValuePtr));
				return true;
			case EFormulaValueType::Int16:
				OutResultValue = MakeShared<FFormulaValue>(static_cast<int16>(FromValuePtr));
				return true;
			case EFormulaValueType::Int32:
				OutResultValue = MakeShared<FFormulaValue>(static_cast<int32>(FromValuePtr));
				return true;
			case EFormulaValueType::Int64:
				OutResultValue = MakeShared<FFormulaValue>(static_cast<int64>(FromValuePtr));
				return true;
			case EFormulaValueType::UInt8:
				OutResultValue = MakeShared<FFormulaValue>(static_cast<uint8>(FromValuePtr));
				return true;
			case EFormulaValueType::UInt16:
				OutResultValue = MakeShared<FFormulaValue>(static_cast<uint16>(FromValuePtr));
				return true;
			case EFormulaValueType::UInt32:
				OutResultValue = MakeShared<FFormulaValue>(static_cast<uint32>(FromValuePtr));
				return true;
			case EFormulaValueType::UInt64:
				OutResultValue = MakeShared<FFormulaValue>(static_cast<uint64>(FromValuePtr));
				return true;
			case EFormulaValueType::Float:
				OutResultValue = MakeShared<FFormulaValue>(static_cast<float>(FromValuePtr));
				return true;
			case EFormulaValueType::Double:
				OutResultValue = MakeShared<FFormulaValue>(static_cast<double>(FromValuePtr));
				return true;
			default: break;
			}
		}
		else if constexpr (std::is_same_v<InT, FString>)
		{
			switch (ToTypeCode)
			{
			case EFormulaValueType::String:
				OutResultValue = FromValue;
				return true;
			case EFormulaValueType::Text:
				OutResultValue = MakeShared<FFormulaValue>(FText::FromString(FromValuePtr));
				return true;
			case EFormulaValueType::Name:
				OutResultValue = MakeShared<FFormulaValue>(FName(FromValuePtr));
				return true;
			default: break;
			}
		}
		else if constexpr (std::is_same_v<InT, FName>)
		{
			switch (ToTypeCode)
			{
			case EFormulaValueType::String:
				OutResultValue = MakeShared<FFormulaValue>(FromValuePtr.ToString());
				return true;
			case EFormulaValueType::Text:
				OutResultValue = MakeShared<FFormulaValue>(FText::FromName(FromValuePtr));
				return true;
			case EFormulaValueType::Name:
				OutResultValue = FromValue;
				return true;
			default: break;
			}
		}
		else if constexpr (std::is_same_v<InT, FText>)
		{
			switch (ToTypeCode)
			{
			case EFormulaValueType::String:
				OutResultValue = MakeShared<FFormulaValue>(FromValuePtr.ToString());
				return true;
			case EFormulaValueType::Text:
				OutResultValue = FromValue;
				return true;
			case EFormulaValueType::Name:
				OutResultValue = MakeShared<FFormulaValue>(FName(FromValuePtr.ToString()));
				return true;
			default: break;
			}
		}
		
		return false;
	});
}
