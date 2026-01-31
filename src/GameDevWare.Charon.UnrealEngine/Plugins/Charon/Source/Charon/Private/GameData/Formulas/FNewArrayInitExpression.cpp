// Copyright GameDevWare, Denis Zykov 2025

#include "GameData/Formulas/Expressions/FNewArrayInitExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/IFormulaType.h"
#include "GameData/Formulas/Expressions/FNewExpression.h"

DEFINE_LOG_CATEGORY(FNLogNewArrayInitExpression);

FNewArrayInitExpression::FNewArrayInitExpression(const TSharedRef<FJsonObject>& ExpressionObj):
	ArrayType(FExpressionBuildHelper::GetTypeRef(ExpressionObj, FFormulaNotation::TYPE_ATTRIBUTE)),
	Initializers(FExpressionBuildHelper::GetArgumentsList(ExpressionObj, FFormulaNotation::INITIALIZERS_ATTRIBUTE))
{}

FNewArrayInitExpression::FNewArrayInitExpression(const TSharedPtr<FFormulaTypeReference>& ArrayType,
	const TArray<TSharedPtr<FFormulaExpression>>& Initializers) : ArrayType(ArrayType), Initializers(Initializers) 
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
	const auto ElementType = Context.TypeResolver->FindType(ElementTypeReference);
	if (!ElementType.IsValid())
	{
		return FFormulaExecutionError::UnableToResolveType(ElementTypeReference->GetFullName(/* include generics */ true));
	}

	if (ExpectedArrayProperty && !ElementType->IsAssignableFrom(ExpectedArrayProperty->Inner, /*bWithoutConversion*/ true))
	{
		ExpectedArrayProperty = nullptr; // expected array type is wrong
		ExpectedElementProperty = nullptr;
	}
	
	TArray<TSharedRef<FFormulaValue>> AddValues;
	for (const auto Initializer : this->Initializers)
	{
		const auto ValueResult = Initializer->Execute(Context, ExpectedElementProperty);
		if (ValueResult.HasError())
		{
			return ValueResult; // propagate error
		}
		AddValues.Add(ValueResult.GetValue());
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

	FArrayProperty* ArrayProperty = CastFieldChecked<FArrayProperty>(ArrayValue->GetType());
	FProperty* ElementProperty = ArrayProperty->Inner;
	FScriptArrayHelper ArrayWrap(ArrayProperty, ArrayValuePtr);
	
	for (const auto ValueToAdd : AddValues)
	{
		const int32 AddedIndex = ArrayWrap.AddUninitializedValue();
		void* ElementPtr = ArrayWrap.GetElementPtr(AddedIndex);
		ElementProperty->InitializeValue(ElementPtr);

		if (!ValueToAdd->TryCopyCompleteValue(ElementProperty, ElementPtr))
		{
			ArrayWrap.RemoveValues(AddedIndex, 1);
			
			return FFormulaExecutionError::CollectionAddFailed(this->ArrayType->GetFullName(/*bWriteGenerics*/ true), ValueToAdd->GetCPPType());
		}
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
