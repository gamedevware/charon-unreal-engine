// Copyright GameDevWare, Denis Zykov 2025

#include "GameData/Formulas/Expressions/FNewArrayInitExpression.h"
#include "GameData/Formulas/Expressions/FNewArrayBoundExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/IFormulaType.h"

DEFINE_LOG_CATEGORY(FNLogNewArrayInitExpression);

FNewArrayInitExpression::FNewArrayInitExpression(const TSharedRef<FJsonObject>& ExpressionObj):
	Initializers(FExpressionBuildHelper::GetArgumentsList(ExpressionObj, FFormulaNotation::ARGUMENTS_ATTRIBUTE)),
	ArrayType(FExpressionBuildHelper::GetTypeRef(ExpressionObj, FFormulaNotation::TYPE_ATTRIBUTE))
{}

FNewArrayInitExpression::FNewArrayInitExpression(const TSharedPtr<FFormulaTypeReference>& ArrayType,
	const TArray<TSharedPtr<FFormulaExpression>>& Initializers) : Initializers(Initializers), ArrayType(ArrayType) 
{
}

FFormulaExecutionResult FNewArrayInitExpression::Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const
{
	if (!this->IsValid())
	{
		return FFormulaExecutionError::ExpressionIsInvalid();
	}
	
	FArrayProperty* ExpectedArrayProperty = CastField<FArrayProperty>(ExpectedType);
	FProperty* ExpectedElementProperty = ExpectedArrayProperty ? ExpectedArrayProperty->Inner : nullptr;
	
	const auto ElementTypeReference = this->ArrayType->TypeArguments[0];
	const auto ElementType = Context.TypeResolver->GetTypeDescription(ElementTypeReference);
	if (!ElementType.IsValid())
	{
		return FFormulaExecutionError::UnableToResolveType(ElementTypeReference->GetFullName(/* include generics */ true));
	}

	if (ExpectedArrayProperty && !ElementType->IsAssignableFrom(ExpectedArrayProperty->Inner, /*bWithoutConversion*/ true))
	{
		ExpectedArrayProperty = nullptr; // expected array type is wrong
		ExpectedElementProperty = nullptr;
	}
	
	TArray<TSharedRef<FFormulaValue>> InitializationValues;
	for (const auto Initializer : this->Initializers)
	{
		const auto ValueResult = Initializer->Execute(Context, ExpectedElementProperty);
		if (ValueResult.HasError())
		{
			return ValueResult; // propagate error
		}
		InitializationValues.Add(ValueResult.GetValue());
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
	if (!FNewArrayBoundExpression::TryCreateArray(ElementType, ExpectedArrayProperty, ArrayValue) ||
		!ArrayValue->TryGetContainerAddress(ArrayValuePtr))
	{
		return FFormulaExecutionError::UnsupportedArrayType(this->ArrayType->GetFullName(/*bWriteGenerics*/ true));
	}
	check(ArrayValuePtr);

	FArrayProperty* ArrayProperty = CastFieldChecked<FArrayProperty>(ArrayValue->GetType());
	FScriptArrayHelper ArrayWrap(ArrayProperty, ArrayValuePtr);
	
	ArrayWrap.EmptyAndAddUninitializedValues(InitializationValues.Num());
	int32 Index = 0;
	for (const auto InitializationValue : InitializationValues)
	{
		if (InitializationValue->TryCopyCompleteValue(ArrayProperty->Inner, ArrayWrap.GetElementPtr(Index)))
		{
			return FFormulaExecutionError::CollectionAddFailed(this->ArrayType->GetFullName(/*bWriteGenerics*/ true), InitializationValue->GetCPPType());
		}
		Index++;
	}

	return ArrayValue;
}

bool FNewArrayInitExpression::IsValid() const
{
	if (!this->ArrayType.IsValid())
	{
		return false;
	}
	for (auto InitializationValue : this->Initializers)
	{
		if (!InitializationValue.IsValid())
		{
			return false;
		}
	}
	return true;
}

void FNewArrayInitExpression::DebugPrintTo(FString& OutValue) const
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
	OutValue.Append("[").AppendInt(this->Initializers.Num());
	OutValue.Append("] { ");
	bool bFirstArgument = true;;
	for (auto InitializationValue : this->Initializers)
	{
		if (!bFirstArgument)
		{
			OutValue.Append(", ");
		}
		bFirstArgument = false;

		if (InitializationValue.IsValid())
		{
			InitializationValue->DebugPrintTo(OutValue);
		}
		else
		{
			OutValue.Append(TEXT("#INVALID#"));
		}
	}
	OutValue.Append("}");
}
