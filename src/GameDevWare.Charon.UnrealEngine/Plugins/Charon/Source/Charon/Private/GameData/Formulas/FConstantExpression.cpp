#include "Misc/CString.h"
#include "GameData/Formulas/FConstantExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaConstants.h"
#include "GameData/Formulas/IFormulaTypeDescription.h"

DEFINE_LOG_CATEGORY(LogConstantExpression);

FConstantExpression::FConstantExpression(const TSharedRef<FJsonObject>& ExpressionObj)
{
	Type = FExpressionBuildHelper::GetTypeRef(ExpressionObj, FormulaConstants::TYPE_ATTRIBUTE);
	Value = FFormulaValue::Null();

	TSharedPtr<FJsonValue> ValueField = ExpressionObj->TryGetField(FormulaConstants::VALUE_ATTRIBUTE);
	if (ValueField.IsValid())
	{
		if (bool bValue; ValueField->TryGetBool(bValue))
		{
			Value = FFormulaValue::Create(bValue);
		}
		if (double DoubleValue; ValueField->TryGetNumber(DoubleValue))
		{
			Value = FFormulaValue::Create(DoubleValue);
		}
		if (FString StringValue; ValueField->TryGetString(StringValue))
		{
			Value = FFormulaValue::Create(StringValue);
		}
	}
}

FFormulaInvokeResult FConstantExpression::Invoke(const FFormulaExecutionContext& Context)
{
	const auto ToType = Context.TypeResolver->GetTypeDescription(this->Type);
	if (!ToType)
	{
		return MakeErrorResult(FFormulaInvokeError::UnableToResolveType(this->Type->GetFullName(/* include generics */ true)));
	}
	if (ToType->CanBeNull() && this->Value.IsNull())
	{
		return MakeValueResult(FFormulaValue::Null());
	}

	const EFormulaValueType FromTypeCode = this->Value.GetTypeCode();
	const EFormulaValueType ToTypeCode = ToType->GetTypeCode();
	double DoubleValue = 0;
	FString StringValue;
	
	switch (FromTypeCode)
	{
	case EFormulaValueType::Boolean:

		// @formatter:off
		switch (ToTypeCode)
		{
			case EFormulaValueType::Boolean: return MakeValueResult(this->Value);
			case EFormulaValueType::String: return MakeValueResult(FFormulaValue::Create(this->Value.ToString()));
			case EFormulaValueType::Name: return MakeValueResult(FFormulaValue::Create(FName(this->Value.ToString())));
			case EFormulaValueType::Text: return MakeValueResult(FFormulaValue::Create(FText::FromString(this->Value.ToString())));
			default: break;
		}
		// @formatter:on

	case EFormulaValueType::Double:
		this->Value.TryCopyCompleteValue(this->Value.GetType(), &DoubleValue);

		// @formatter:off
		switch (ToTypeCode)
		{
			case EFormulaValueType::Boolean: return MakeValueResult(FFormulaValue::Create(DoubleValue != 0));
			case EFormulaValueType::UInt8: return   MakeValueResult(FFormulaValue::Create(static_cast<uint32>(DoubleValue)));
			case EFormulaValueType::UInt16: return  MakeValueResult(FFormulaValue::Create(static_cast<uint32>(DoubleValue)));
			case EFormulaValueType::UInt32: return  MakeValueResult(FFormulaValue::Create(static_cast<uint32>(DoubleValue)));
			case EFormulaValueType::UInt64: return  MakeValueResult(FFormulaValue::Create(static_cast<uint64>(DoubleValue)));
			case EFormulaValueType::Int8: return    MakeValueResult(FFormulaValue::Create(static_cast<int32>(DoubleValue)));
			case EFormulaValueType::Int16: return   MakeValueResult(FFormulaValue::Create(static_cast<int32>(DoubleValue)));
			case EFormulaValueType::Int32: return   MakeValueResult(FFormulaValue::Create(static_cast<int32>(DoubleValue)));
			case EFormulaValueType::Int64: return   MakeValueResult(FFormulaValue::Create(static_cast<int64>(DoubleValue)));
			case EFormulaValueType::Float: return   MakeValueResult(FFormulaValue::Create(static_cast<float>(DoubleValue)));
			case EFormulaValueType::Double: return  MakeValueResult(this->Value);
			case EFormulaValueType::String: return  MakeValueResult(FFormulaValue::Create(this->Value.ToString()));
			case EFormulaValueType::Name: return    MakeValueResult(FFormulaValue::Create(FName(this->Value.ToString())));
			case EFormulaValueType::Text: return    MakeValueResult(FFormulaValue::Create(FText::FromString(this->Value.ToString())));
			default: break;
		}
		// @formatter:on
		
	case EFormulaValueType::String:
		StringValue = this->Value.ToString();
		
		// @formatter:off
		switch (ToTypeCode)
		{
			case EFormulaValueType::Boolean:return MakeValueResult(FFormulaValue::Create(TCString<FString::ElementType>::ToBool(*StringValue)));
			case EFormulaValueType::UInt8:
			case EFormulaValueType::UInt16:
			case EFormulaValueType::UInt32: return MakeValueResult(FFormulaValue::Create(static_cast<uint32>(TCString<FString::ElementType>::Strtoui64(*StringValue, nullptr, 0))));
			case EFormulaValueType::UInt64: return MakeValueResult(FFormulaValue::Create(TCString<FString::ElementType>::Strtoui64(*StringValue, nullptr, 0)));
			case EFormulaValueType::Int8:
			case EFormulaValueType::Int16:
			case EFormulaValueType::Int32: return MakeValueResult(FFormulaValue::Create(TCString<FString::ElementType>::Atoi(*StringValue)));
			case EFormulaValueType::Int64: return MakeValueResult(FFormulaValue::Create(TCString<FString::ElementType>::Atoi64(*StringValue)));
			case EFormulaValueType::Float: return MakeValueResult(FFormulaValue::Create(TCString<FString::ElementType>::Atof(*StringValue)));
			case EFormulaValueType::Double: return MakeValueResult(FFormulaValue::Create(TCString<FString::ElementType>::Atod(*StringValue)));
			case EFormulaValueType::String: return MakeValueResult(this->Value);
			case EFormulaValueType::Name: return MakeValueResult(FFormulaValue::Create(FName(this->Value.ToString())));
			case EFormulaValueType::Text: return MakeValueResult(FFormulaValue::Create(FText::FromString(this->Value.ToString())));
			default: break;
		}
		// @formatter:on

	default: break;
	}

	return MakeErrorResult(
		FFormulaInvokeError::CantConvertToType(this->Value.GetType()->GetCPPType(), ToType->GetCPPType()));
}
