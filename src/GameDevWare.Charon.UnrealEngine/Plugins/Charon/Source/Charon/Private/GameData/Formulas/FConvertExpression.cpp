#include "GameData/Formulas/FConvertExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaConstants.h"
#include "GameData/Formulas/IFormulaTypeDescription.h"
#include "GameData/Formulas/FFormulaTypeReference.h"

FConvertExpression::FConvertExpression(const TSharedRef<FJsonObject>& ExpressionObj)
{
	ExpressionObj->TryGetStringField(FormulaConstants::EXPRESSION_TYPE_ATTRIBUTE, ExpressionType);
	Type = FExpressionBuildHelper::GetTypeRef(ExpressionObj, FormulaConstants::TYPE_ATTRIBUTE);
	Expression = FExpressionBuildHelper::GetExpression(ExpressionObj, FormulaConstants::EXPRESSION_ATTRIBUTE);
}

static TArray<UClass*> EmptyTypeArgument;
FFormulaInvokeResult FConvertExpression::Invoke(const FFormulaExecutionContext& Context)
{
	const auto Result = this->Expression->Invoke(Context);
	if (Result.IsType<FFormulaInvokeError>())
	{
		return Result; // propagate error
	}

	const auto FromValue = Result.Get<FFormulaValue>();
	const auto ToType = Context.TypeResolver->GetTypeDescription(this->Type);
	if (!ToType)
	{
		return MakeErrorResult(FFormulaInvokeError::UnableToResolveType(this->Type->GetFullName(/* include generics */ true)));
	}
	
	const auto ToTypeCode = ToType->GetTypeCode();
	
	if (FromValue.IsNull())
	{
		if (ToType->CanBeNull())
		{
			return Result;
		}
		else
		{
			return MakeErrorResult(
				FFormulaInvokeError::CanConvertNullToType(ToType->GetCPPType())
			);
		}
	}
	
	if (const FObjectPropertyBase* FromObjectProp = CastField<FObjectPropertyBase>(FromValue.GetType());
		ToTypeCode == EFormulaValueType::ObjectPtr)
	{
		if (ToType->IsAssignableFrom(FromObjectProp->PropertyClass))
		{
			return Result; // UClass cast success
		}
		if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_TYPE_AS)
		{
			// this is not an error in `x as ClassName` expression
			return MakeValueResult(FFormulaValue::Null()); // UClass cast failure
		}
		else
		{
			return MakeErrorResult(
				FFormulaInvokeError::InvalidCastError(FromValue.GetType()->GetCPPType(), ToType->GetCPPType()));
		}
	}

	FFormulaMethodGroup* ConversionOperation;
	FFormulaValue ResultValue;
	const TMap<FString, FFormulaValue> ConversionArguments {
		{ TEXT("0"), FromValue }
	};
	if (ToType->TryGetConversionOperation(ConversionOperation) &&
		ConversionOperation &&
		ConversionOperation->TryInvoke(
			FFormulaValue::Null(), ConversionArguments, ToType.Get(), EmptyTypeArgument, ResultValue
		))
	{
		return MakeValueResult(ResultValue);
	}

	return MakeErrorResult(
		FFormulaInvokeError::CantConvertToType(FromValue.GetType()->GetCPPType(), ToType->GetCPPType()));
}
