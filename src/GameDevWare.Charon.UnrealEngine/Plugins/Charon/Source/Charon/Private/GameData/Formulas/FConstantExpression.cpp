// Copyright GameDevWare, Denis Zykov 2025

#include "GameData/Formulas/Expressions/FConstantExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/IFormulaType.h"
#include "Misc/CString.h"

DEFINE_LOG_CATEGORY(LogConstantExpression);

static TSharedRef<FFormulaValue> GetConstantValue(const TSharedRef<FJsonObject>& ExpressionObj)
{
	TSharedPtr<FFormulaValue> Value = FFormulaValue::Null();
	TSharedPtr<FJsonValue> ValueField = ExpressionObj->TryGetField(FFormulaNotation::VALUE_ATTRIBUTE);
	if (ValueField.IsValid())
	{
		if (bool bValue; ValueField->TryGetBool(bValue))
		{
			Value = MakeShared<FFormulaValue>(bValue);
		}
		if (double DoubleValue; ValueField->TryGetNumber(DoubleValue))
		{
			Value = MakeShared<FFormulaValue>(DoubleValue);
		}
		if (FString StringValue; ValueField->TryGetString(StringValue))
		{
			Value = MakeShared<FFormulaValue>(StringValue);
		}
	}
	return Value.ToSharedRef();
}

FConstantExpression::FConstantExpression(const TSharedRef<FJsonObject>& ExpressionObj):
	ValueType(FExpressionBuildHelper::GetTypeRef(ExpressionObj, FFormulaNotation::TYPE_ATTRIBUTE)),
	Value(GetConstantValue(ExpressionObj))
{
}

FConstantExpression::FConstantExpression(const TSharedRef<FFormulaValue>& Value,
	const TSharedRef<FFormulaTypeReference>& ValueType) : ValueType(ValueType), Value(Value)
{
}

FFormulaExecutionResult FConstantExpression::Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const
{
	if (!this->IsValid())
	{
		return FFormulaExecutionError::ExpressionIsInvalid();
	}
	
	const auto ToType = Context.TypeResolver->FindType(this->ValueType);
	if (!ToType)
	{
		return FFormulaExecutionError::UnableToResolveType(this->ValueType->GetFullName(/* include generics */ true));
	}
	if (ToType->CanBeNull() && this->Value->IsNull())
	{
		return FFormulaValue::Null();
	}

	const EFormulaValueType FromTypeCode = this->Value->GetTypeCode();
	const EFormulaValueType ToTypeCode = ToType->GetTypeCode();

	if (FromTypeCode == ToTypeCode)
	{
		return this->Value;
	}

	// TODO lift constant to expected type?
	
	double DoubleValue = 0;
	FString StringValue;
	switch (FromTypeCode)
	{
	case EFormulaValueType::Boolean:

		// @formatter:off
		switch (ToTypeCode)
		{
			case EFormulaValueType::Boolean: return this->Value;
			case EFormulaValueType::String: return this->Value->ToString();
			case EFormulaValueType::Name: return FName(this->Value->ToString());
			case EFormulaValueType::Text: return FText::FromString(this->Value->ToString());
			default: break;
		}
		// @formatter:on

	case EFormulaValueType::Double:
		this->Value->TryCopyCompleteValue(this->Value->GetType(), &DoubleValue);

		// @formatter:off
		switch (ToTypeCode)
		{
			case EFormulaValueType::Boolean: return DoubleValue != 0;
			case EFormulaValueType::UInt8:   return MakeShared<FFormulaValue>(static_cast<uint8>(DoubleValue));
			case EFormulaValueType::UInt16:  return MakeShared<FFormulaValue>(static_cast<uint16>(DoubleValue));
			case EFormulaValueType::UInt32:  return MakeShared<FFormulaValue>(static_cast<uint32>(DoubleValue));
			case EFormulaValueType::UInt64:  return static_cast<uint64>(DoubleValue);
			case EFormulaValueType::Int8:    return MakeShared<FFormulaValue>(static_cast<int8>(DoubleValue));
			case EFormulaValueType::Int16:   return MakeShared<FFormulaValue>(static_cast<int16>(DoubleValue));
			case EFormulaValueType::Int32:   return MakeShared<FFormulaValue>(static_cast<int32>(DoubleValue));
			case EFormulaValueType::Int64:   return static_cast<int64>(DoubleValue);
			case EFormulaValueType::Float:   return static_cast<float>(DoubleValue);
			case EFormulaValueType::Double:  return this->Value;
			case EFormulaValueType::String:  return this->Value->ToString();
			case EFormulaValueType::Name:    return FName(this->Value->ToString());
			case EFormulaValueType::Text:    return FText::FromString(this->Value->ToString());
			default: break;
		}
		// @formatter:on
		
	case EFormulaValueType::String:
		StringValue = this->Value->ToString();
		
		// @formatter:off
		switch (ToTypeCode)
		{
			case EFormulaValueType::Boolean:  return TCString<FString::ElementType>::ToBool(*StringValue);
			case EFormulaValueType::UInt8:    return MakeShared<FFormulaValue>(static_cast<uint8>(TCString<FString::ElementType>::Strtoui64(*StringValue, nullptr, 0)));
			case EFormulaValueType::UInt16:   return MakeShared<FFormulaValue>(static_cast<uint16>(TCString<FString::ElementType>::Strtoui64(*StringValue, nullptr, 0)));
			case EFormulaValueType::UInt32:   return MakeShared<FFormulaValue>(static_cast<uint32>(TCString<FString::ElementType>::Strtoui64(*StringValue, nullptr, 0)));
			case EFormulaValueType::UInt64:   return TCString<FString::ElementType>::Strtoui64(*StringValue, nullptr, 0);
			case EFormulaValueType::Int8:     return MakeShared<FFormulaValue>(static_cast<int8>(TCString<FString::ElementType>::Strtoi64(*StringValue, nullptr, 0)));
			case EFormulaValueType::Int16:    return MakeShared<FFormulaValue>(static_cast<int16>(TCString<FString::ElementType>::Strtoi64(*StringValue, nullptr, 0)));
			case EFormulaValueType::Int32:    return MakeShared<FFormulaValue>(TCString<FString::ElementType>::Atoi(*StringValue));
			case EFormulaValueType::Int64:    return TCString<FString::ElementType>::Atoi64(*StringValue);
			case EFormulaValueType::Float:    return TCString<FString::ElementType>::Atof(*StringValue);
			case EFormulaValueType::Double:   return TCString<FString::ElementType>::Atod(*StringValue);
			case EFormulaValueType::String:   return this->Value;
			case EFormulaValueType::Name:     return FName(this->Value->ToString());
			case EFormulaValueType::Text:     return FText::FromString(this->Value->ToString());
			default: break;
		}
		// @formatter:on

	default: break;
	}

	return 	FFormulaExecutionError::CantConvertToType(this->Value->GetCPPType(), ToType->GetCPPType());
}

bool FConstantExpression::IsValid() const
{
	return this->ValueType.IsValid();
}

void FConstantExpression::DebugPrintTo(FString& OutValue) const
{
	switch (this->Value->GetTypeCode()) {
	case EFormulaValueType::Null: OutValue.Append(TEXT("null")); break;
	case EFormulaValueType::Boolean:
	case EFormulaValueType::UInt8:
	case EFormulaValueType::UInt16:
	case EFormulaValueType::UInt32:
	case EFormulaValueType::UInt64:
	case EFormulaValueType::Int8:
	case EFormulaValueType::Int16:
	case EFormulaValueType::Int32:
	case EFormulaValueType::Int64:
	case EFormulaValueType::Float:
	case EFormulaValueType::Double: OutValue.Append(this->Value->ToString()); break;
	case EFormulaValueType::Timespan:
	case EFormulaValueType::DateTime:
	case EFormulaValueType::String:
	case EFormulaValueType::Name:
	case EFormulaValueType::Text: OutValue.Append(TEXT("\"")).Append(this->Value->ToString()).Append(TEXT("\"")); break;
	case EFormulaValueType::ObjectPtr:
	case EFormulaValueType::Struct:
	case EFormulaValueType::Enum:
	default: OutValue.Append(TEXT("#")).Append(this->Value->ToString()).Append(TEXT("#")); break;
	}

}
