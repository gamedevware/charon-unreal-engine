// Copyright GameDevWare, Denis Zykov 2025

#include "GameData/Formulas/Expressions/FDefaultExpression.h"
#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/IFormulaType.h"
#include "Misc/DateTime.h"
#include "Misc/Timespan.h"
#include "Templates/SharedPointer.h"
#include "UObject/NameTypes.h"
#include "UObject/Object.h"

FDefaultExpression::FDefaultExpression(const TSharedRef<FJsonObject>& ExpressionObj):
	ValueType(FExpressionBuildHelper::GetTypeRef(ExpressionObj, FFormulaNotation::TYPE_ATTRIBUTE))
{}

FDefaultExpression::FDefaultExpression(const TSharedPtr<FFormulaTypeReference>& ValueType) : ValueType(ValueType)
{
}

FFormulaExecutionResult FDefaultExpression::Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const
{
	if (!this->IsValid())
	{
		return FFormulaExecutionError::ExpressionIsInvalid();
	}
	
	const auto ToType = Context.TypeResolver->GetTypeDescription(this->ValueType);
	if (!ToType.IsValid())
	{
		return FFormulaExecutionError::UnableToResolveType(this->ValueType->GetFullName(/* include generics */ true));
	}

	switch (ToType->GetTypeCode()) {
	case EFormulaValueType::Null: return FFormulaValue::Null();
	case EFormulaValueType::Boolean: return FFormulaValue::FalseBool();
	case EFormulaValueType::UInt8: return MakeShared<FFormulaValue>(static_cast<uint8>(0));
	case EFormulaValueType::UInt16: return MakeShared<FFormulaValue>(static_cast<uint16>(0));
	case EFormulaValueType::UInt32: return MakeShared<FFormulaValue>(static_cast<uint32>(0));
	case EFormulaValueType::UInt64: return MakeShared<FFormulaValue>(static_cast<uint64>(0));
	case EFormulaValueType::Int8: return MakeShared<FFormulaValue>(static_cast<int8>(0));
	case EFormulaValueType::Int16: return MakeShared<FFormulaValue>(static_cast<int16>(0));
	case EFormulaValueType::Int32: return MakeShared<FFormulaValue>(static_cast<int32>(0));
	case EFormulaValueType::Int64: return MakeShared<FFormulaValue>(static_cast<int64>(0));
	case EFormulaValueType::Float: return MakeShared<FFormulaValue>(static_cast<float>(0));
	case EFormulaValueType::Double: return MakeShared<FFormulaValue>(static_cast<double>(0));
	case EFormulaValueType::Timespan: return FTimespan();
	case EFormulaValueType::DateTime: return FDateTime();
	case EFormulaValueType::String: return FString();
	case EFormulaValueType::Name: return FName();
	case EFormulaValueType::Text: return FText();
	case EFormulaValueType::Enum: return 0;
	case EFormulaValueType::ObjectPtr: return FFormulaValue::Null();
	case EFormulaValueType::Struct:
	default: return FFormulaExecutionError::UnsupportedObjectType(ToType->GetCPPType());
	
	}
}

bool FDefaultExpression::IsValid() const
{
	return this->ValueType.IsValid();
}

void FDefaultExpression::DebugPrintTo(FString& OutValue) const
{
	OutValue.Append(TEXT("default("));
	if (this->ValueType.IsValid())
	{
		OutValue.Append(this->ValueType->GetFullName(true));
	}
	else
	{
		OutValue.Append(TEXT("#INVALID#"));
	}
	OutValue.Append(TEXT(") "));

}
