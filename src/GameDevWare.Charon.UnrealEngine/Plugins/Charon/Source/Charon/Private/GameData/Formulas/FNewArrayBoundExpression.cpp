// Copyright GameDevWare, Denis Zykov 2025

#include "GameData/Formulas/Expressions/FNewArrayBoundExpression.h"

#include "GameData/Formulas/DotNetTypes/UDotNetArray.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/IFormulaType.h"
#include "GameData/Formulas/Expressions/FNewExpression.h"

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
	const auto ElementType = Context.TypeResolver->FindType(ElementTypeReference);
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
	if (!FNewExpression::TryCreateArray(ElementType, ExpectedArrayProperty, ArrayValue) ||
		!ArrayValue->TryGetContainerAddress(ArrayValuePtr))
	{
		return FFormulaExecutionError::MissingContextForStruct(this->ArrayType->GetFullName(/*bWriteGenerics*/ true));
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