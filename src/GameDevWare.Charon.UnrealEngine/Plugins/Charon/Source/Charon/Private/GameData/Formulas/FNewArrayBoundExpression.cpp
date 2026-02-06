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
	ElementTypeRef(FExpressionBuildHelper::GetTypeRef(ExpressionObj, FFormulaNotation::TYPE_ATTRIBUTE))
{}

FNewArrayBoundExpression::FNewArrayBoundExpression(const TSharedPtr<FFormulaTypeReference>& ElementType,
	const TMap<FString, TSharedPtr<FFormulaExpression>>& Arguments) : Arguments(Arguments), ElementTypeRef(ElementType) 
{
}

FFormulaExecutionResult FNewArrayBoundExpression::Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const
{
	if (!this->IsValid())
	{
		return FFormulaExecutionError::ExpressionIsInvalid();
	}
	
	FArrayProperty* ExpectedArrayProperty = CastField<FArrayProperty>(ExpectedType);
	const auto ElementType = Context.TypeResolver->FindType(this->ElementTypeRef);
	if (!ElementType.IsValid())
	{
		return FFormulaExecutionError::UnableToResolveType(this->ElementTypeRef->GetFullName(/* include generics */ true));
	}

	if (ExpectedArrayProperty && !ElementType->IsAssignableFrom(ExpectedArrayProperty->Inner, /*bWithoutConversion*/ true))
	{
		ExpectedArrayProperty = nullptr; // expected array type is wrong
	}
	
	int32 ArraySize = 0;
	for (const auto& ArgumentPair : this->Arguments)
	{
		const auto ArgumentResult = ArgumentPair.Value->Execute(Context, UDotNetInt32::GetLiteralProperty());
		if (ArgumentResult.HasError())
		{
			return ArgumentResult; // propagate error
		}
		const auto& ArgumentValue = ArgumentResult.GetValue();
		if (!ArgumentValue->TryGetInt32(ArraySize))
		{
			return FFormulaExecutionError::InvalidArraySizeValue(ArgumentValue->GetCPPType());
		}
		break;
	}
	
	TSharedPtr<FFormulaValue> ArrayValue = FFormulaValue::Null();
	void* ArrayValuePtr = nullptr;
	if (!FNewExpression::TryCreateArray(ElementType, ExpectedArrayProperty, ArrayValue) ||
		!ArrayValue->TryGetContainerAddress(ArrayValuePtr))
	{
		return FFormulaExecutionError::MissingContextForStruct(this->ElementTypeRef->GetFullName(/*bWriteGenerics*/ true));
	}
	check(ArrayValuePtr);
	
	FScriptArrayHelper ArrayWrap(CastFieldChecked<FArrayProperty>(ArrayValue->GetType()), ArrayValuePtr);
	
	ArrayWrap.EmptyAndAddValues(ArraySize);

	return  ArrayValue;
}

bool FNewArrayBoundExpression::IsValid() const
{
	if (!this->ElementTypeRef.IsValid())
	{
		return false;
	}
	for (const auto& ArgumentPair : this->Arguments)
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
	if (this->ElementTypeRef.IsValid())
	{
		OutValue.Append(this->ElementTypeRef->GetFullName(true));
	}
	else
	{
		OutValue.Append(TEXT("#INVALID#"));
	}
	OutValue.Append("[");
	bool bFirstArgument = true;;
	for (const auto& ArgumentPair : this->Arguments)
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