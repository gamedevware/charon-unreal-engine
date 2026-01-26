// Copyright GameDevWare, Denis Zykov 2025

#include "GameData/Formulas/Expressions/FNewArrayBoundExpression.h"

#include "GameData/Formulas/DotNetTypes/UDotNetArray.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/IFormulaType.h"

DEFINE_LOG_CATEGORY(LogNewArrayBoundExpression);

FNewArrayBoundExpression::FNewArrayBoundExpression(const TSharedRef<FJsonObject>& ExpressionObj):
	Arguments(FExpressionBuildHelper::GetArguments(ExpressionObj, FFormulaNotation::ARGUMENTS_ATTRIBUTE)),
	ArrayType(FExpressionBuildHelper::GetTypeRef(ExpressionObj, FFormulaNotation::TYPE_ATTRIBUTE))
{}

FNewArrayBoundExpression::FNewArrayBoundExpression(const TSharedPtr<FFormulaTypeReference>& ArrayType,
	const TMap<FString, TSharedPtr<FFormulaExpression>>& Arguments) : Arguments(Arguments), ArrayType(ArrayType) 
{
}

FFormulaExecutionResult FNewArrayBoundExpression::Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const
{
	if (!this->IsValid())
	{
		return FFormulaExecutionError::ExpressionIsInvalid();
	}

	if (this->Arguments.Num() != 1)
	{
		UE_LOG(LogNewArrayBoundExpression, Error, TEXT("'%s': array 'new' expression must have exactly one argument (found %d)."), *this->ArrayType->GetFullName(true), this->Arguments.Num());
	}

	FArrayProperty* ExpectedArrayProperty = CastField<FArrayProperty>(ExpectedType);
	const auto ElementTypeReference = this->ArrayType->TypeArguments[0];
	const auto ElementType = Context.TypeResolver->GetTypeDescription(ElementTypeReference);
	if (!ElementType.IsValid())
	{
		return FFormulaExecutionError::UnableToResolveType(ElementTypeReference->GetFullName(/* include generics */ true));
	}

	if (ExpectedArrayProperty && !ElementType->IsAssignableFrom(ExpectedArrayProperty->Inner, /*bWithoutConversion*/ true))
	{
		ExpectedArrayProperty = nullptr; // expected array type is wrong
	}
	
	int32 ArraySize = 0;
	for (const auto ArgumentPair : this->Arguments)
	{
		const auto ArgumentResult = ArgumentPair.Value->Execute(Context, UDotNetInt32::GetLiteralProperty());
		if (ArgumentResult.HasError())
		{
			return ArgumentResult; // propagate error
		}
		const auto ArgumentValue = ArgumentResult.GetValue();
		if (!ArgumentValue->TryGetInt32(ArraySize))
		{
			return FFormulaExecutionError::InvalidArraySizeValue(ArgumentValue->GetCPPType());
		}
		break;
	}

	FString FullName = this->ArrayType->GetFullName(/*bWriteGenerics*/ false);
	if ((FullName != TEXT("Array") && FullName != TEXT("System.Array")) ||
		ArrayType->TypeArguments.Num() != 1)
	{
		return FFormulaExecutionError::InvalidArrayType(this->ArrayType->GetFullName(/*bWriteGenerics*/ false));
	}

	TSharedPtr<FFormulaValue> ArrayValue = FFormulaValue::Null();
	void* ArrayValuePtr = nullptr;
	if (!TryCreateArray(ElementType, ExpectedArrayProperty, ArrayValue) ||
		!ArrayValue->TryGetContainerAddress(ArrayValuePtr))
	{
		return FFormulaExecutionError::UnsupportedArrayType(this->ArrayType->GetFullName(/*bWriteGenerics*/ true));
	}
	check(ArrayValuePtr);
	
	FScriptArrayHelper ArrayWrap(CastFieldChecked<FArrayProperty>(ArrayValue->GetType()), ArrayValuePtr);
	
	ArrayWrap.EmptyAndAddValues(ArraySize);

	return  ArrayValue;
}

bool FNewArrayBoundExpression::IsValid() const
{
	if (!this->ArrayType.IsValid())
	{
		return false;
	}
	for (auto ArgumentPair : this->Arguments)
	{
		if (!ArgumentPair.Value.IsValid())
		{
			return false;
		}
	}
	return true;
}

void FNewArrayBoundExpression::DebugPrintTo(FString& OutValue) const
{
	OutValue.Append("new ");
	if (this->ArrayType.IsValid())
	{
		OutValue.Append(this->ArrayType->GetFullName(true));
	}
	else
	{
		OutValue.Append(TEXT("#INVALID#"));
	}
	OutValue.Append("[");
	bool bFirstArgument = true;;
	for (auto ArgumentPair : this->Arguments)
	{
		if (!bFirstArgument)
		{
			OutValue.Append(", ");
		}
		bFirstArgument = false;

		const bool bIsNumberKey = ArgumentPair.Key == TEXT("0") || FCString::Atoi(*ArgumentPair.Key) != 0;
		if (!bIsNumberKey)
		{
			OutValue.Append(ArgumentPair.Key).Append(": ");
		}
		if (ArgumentPair.Value.IsValid())
		{
			ArgumentPair.Value->DebugPrintTo(OutValue);
		}
		else
		{
			OutValue.Append(TEXT("#INVALID#"));
		}
	}
	OutValue.Append("]");
}

bool FNewArrayBoundExpression::TryCreateArray(const TSharedPtr<IFormulaType>& ElementType,
	FArrayProperty* ArrayProperty, TSharedPtr<FFormulaValue>& OutCreatedArray)
{
	TArray<uint8> Array;

	if (!ArrayProperty)
	{
		switch (ElementType->GetTypeCode()) {
		case EFormulaValueType::Boolean: ArrayProperty = UDotNetBoolean::GetArrayProperty(); break;
		case EFormulaValueType::UInt8: ArrayProperty = UDotNetUInt8::GetArrayProperty(); break;
		case EFormulaValueType::UInt16: ArrayProperty = UDotNetUInt16::GetArrayProperty(); break;
		case EFormulaValueType::UInt32: ArrayProperty = UDotNetUInt32::GetArrayProperty(); break;
		case EFormulaValueType::UInt64: ArrayProperty = UDotNetUInt64::GetArrayProperty(); break;
		case EFormulaValueType::Int8: ArrayProperty = UDotNetInt8::GetArrayProperty(); break;
		case EFormulaValueType::Int16: ArrayProperty = UDotNetInt16::GetArrayProperty(); break;
		case EFormulaValueType::Int32: ArrayProperty = UDotNetInt32::GetArrayProperty(); break;
		case EFormulaValueType::Int64: ArrayProperty = UDotNetInt64::GetArrayProperty(); break;
		case EFormulaValueType::Float: ArrayProperty = UDotNetSingle::GetArrayProperty(); break;
		case EFormulaValueType::Double: ArrayProperty = UDotNetDouble::GetArrayProperty(); break;
		case EFormulaValueType::Timespan: ArrayProperty = UDotNetTimeSpan::GetArrayProperty(); break;
		case EFormulaValueType::DateTime: ArrayProperty = UDotNetDateTime::GetArrayProperty(); break;
		case EFormulaValueType::String: ArrayProperty = UDotNetString::GetArrayProperty(); break;
		case EFormulaValueType::Name: ArrayProperty = UDotNetName::GetArrayProperty(); break;
		case EFormulaValueType::Text: ArrayProperty = UDotNetText::GetArrayProperty(); break;
		case EFormulaValueType::Null:
		case EFormulaValueType::ObjectPtr: ArrayProperty = UDotNetObject::GetArrayProperty(); break;
		case EFormulaValueType::Struct: 
		case EFormulaValueType::Enum: ArrayProperty = nullptr; break; // not supported
		}
	}

	if (!ArrayProperty)
	{
		return  false;
	}
	
	Array.SetNumZeroed(ArrayProperty->GetSize());
	ArrayProperty->InitializeValue(Array.GetData());
	OutCreatedArray = MakeShared<FFormulaValue>(ArrayProperty, Array.GetData());

	// it was copied by FFormulaValue, so we need to dispose our copy of TArray
	ArrayProperty->DestroyValue(Array.GetData());
	
	return true;
}