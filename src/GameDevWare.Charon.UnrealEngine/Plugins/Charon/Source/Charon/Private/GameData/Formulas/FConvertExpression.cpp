#include "GameData/Formulas/Expressions/FConvertExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/IFormulaTypeDescription.h"
#include "GameData/Formulas/FFormulaTypeReference.h"

FConvertExpression::FConvertExpression(const TSharedRef<FJsonObject>& ExpressionObj):
	ConversionType(FExpressionBuildHelper::GetTypeRef(ExpressionObj, FFormulaNotation::TYPE_ATTRIBUTE)),
	Expression(FExpressionBuildHelper::GetExpression(ExpressionObj, FFormulaNotation::EXPRESSION_ATTRIBUTE)),
	ExpressionType(FExpressionBuildHelper::GetString(ExpressionObj, FFormulaNotation::EXPRESSION_TYPE_ATTRIBUTE))
{}

static TArray<UField*> EmptyTypeArgument;
FFormulaInvokeResult FConvertExpression::Execute(const FFormulaExecutionContext& Context) const
{
	if (!this->Expression.IsValid() || !this->ConversionType.IsValid())
	{
		return FFormulaInvokeError::ExpressionIsInvalid();
	}
	
	const auto Result = this->Expression->Execute(Context);
	if (Result.HasError())
	{
		return Result; // propagate error
	}

	const auto FromValue = Result.GetValue();
	const auto ToType = Context.TypeResolver->GetTypeDescription(this->ConversionType);
	if (!ToType)
	{
		return FFormulaInvokeError::UnableToResolveType(this->ConversionType->GetFullName(/* include generics */ true));
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
			return FFormulaInvokeError::CanConvertNullToType(ToType->GetCPPType());
		}
	}
	
	if (const FObjectPropertyBase* FromObjectProp = CastField<FObjectPropertyBase>(FromValue->GetType());
		ToTypeCode == EFormulaValueType::ObjectPtr)
	{
		if (ToType->IsAssignableFrom(FromObjectProp->PropertyClass))
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
			return FFormulaInvokeError::InvalidCastError(FromValue->GetType()->GetCPPType(), ToType->GetCPPType());
		}
	}

	const FFormulaFunction* ConversionOperation;
	if (ToType->TryGetConversionOperation(ConversionOperation) && ConversionOperation)
	{
		const TMap<FString, TSharedRef<FFormulaValue>> ConversionArguments {
			{ TEXT("0"), FromValue }
		};
		TSharedPtr<FFormulaValue> ResultValue;
		if (ConversionOperation->TryInvoke(
			FFormulaValue::Null(),
			ConversionArguments,
			ToType.Get(),
			EmptyTypeArgument,
			ResultValue
		))
		{
			return ResultValue;
		}
	}

	return FFormulaInvokeError::CantConvertToType(FromValue->GetType()->GetCPPType(), ToType->GetCPPType());
}
