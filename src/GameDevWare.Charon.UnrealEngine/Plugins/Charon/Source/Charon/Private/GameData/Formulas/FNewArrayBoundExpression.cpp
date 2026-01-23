#include "GameData/Formulas/Expressions/FNewArrayBoundExpression.h"
#include "GameData/Formulas/DotNetTypes/FDotNetArray.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/IFormulaTypeDescription.h"
#include "Misc/EngineVersionComparison.h"

DEFINE_LOG_CATEGORY(LogNewArrayBoundExpression);

FNewArrayBoundExpression::FNewArrayBoundExpression(const TSharedRef<FJsonObject>& ExpressionObj):
	Arguments(FExpressionBuildHelper::GetArguments(ExpressionObj, FFormulaNotation::ARGUMENTS_ATTRIBUTE)),
	ArrayType(FExpressionBuildHelper::GetTypeRef(ExpressionObj, FFormulaNotation::TYPE_ATTRIBUTE))
{}

FFormulaInvokeResult FNewArrayBoundExpression::Execute(const FFormulaExecutionContext& Context) const
{
	if (!this->ArrayType.IsValid())
	{
		return FFormulaInvokeError::ExpressionIsInvalid();
	}

	if (this->Arguments.Num() != 1)
	{
		UE_LOG(LogNewArrayBoundExpression, Error, TEXT("'%s': array 'new' expression must have exactly one argument (found %d)."), *this->ArrayType->GetFullName(true), this->Arguments.Num());
	}
	
	int32 ArraySize = 0;
	for (const auto ArgumentPair : this->Arguments)
	{
		const auto ArgumentResult = ArgumentPair.Value->Execute(Context);
		if (ArgumentResult.HasError())
		{
			return ArgumentResult; // propagate error
		}
		const auto ArgumentValue = ArgumentResult.GetValue();
		if (!ArgumentValue->TryGetInt32(ArraySize))
		{
			return FFormulaInvokeError::InvalidArraySizeValue(ArgumentValue->GetType()->GetCPPType());
		}
		break;
	}

	FString FullName = this->ArrayType->GetFullName(/*bWriteGenerics*/ false);
	if ((FullName != TEXT("Array") && FullName != TEXT("System.Array")) ||
		ArrayType->TypeArguments.Num() != 1)
	{
		return FFormulaInvokeError::InvalidArrayType(this->ArrayType->GetFullName(/*bWriteGenerics*/ false));
	}

	const auto ElementTypeReference = this->ArrayType->TypeArguments[0];
	const auto ElementType = Context.TypeResolver->GetTypeDescription(ElementTypeReference);
	if (!ElementType)
	{
		return FFormulaInvokeError::UnableToResolveType(ElementTypeReference->GetFullName(/* include generics */ true));
	}

	const auto ArrayProperty = FDotNetArray::GetLiteralProperty(ElementType->GetTypeCode());
	if (!ArrayProperty)
	{
		return FFormulaInvokeError::UnsupportedArrayType(this->ArrayType->GetFullName(/*bWriteGenerics*/ true));
	}

	TArray<uint8> Array;
#if UE_VERSION_NEWER_THAN(5, 5, -1)
	Array.SetNumZeroed(ArrayProperty->GetElementSize());
#else
	Array.SetNumZeroed(ArrayProperty->ElementSize);
#endif
	ArrayProperty->InitializeValue(Array.GetData());
	
	FScriptArrayHelper ArrayWrap(ArrayProperty, Array.GetData());
	ArrayWrap.EmptyAndAddValues(ArraySize);

	return MakeShared<FFormulaValue>(ArrayProperty, Array.GetData());
}
