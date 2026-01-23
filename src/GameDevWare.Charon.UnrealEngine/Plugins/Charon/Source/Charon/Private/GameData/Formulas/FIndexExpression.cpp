#include "GameData/Formulas/Expressions/FIndexExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "Misc/EngineVersionComparison.h"

FIndexExpression::FIndexExpression(const TSharedRef<FJsonObject>& ExpressionObj) :
	Arguments(FExpressionBuildHelper::GetArguments(ExpressionObj, FFormulaNotation::ARGUMENTS_ATTRIBUTE)),
	Expression(FExpressionBuildHelper::GetExpression(ExpressionObj, FFormulaNotation::EXPRESSION_ATTRIBUTE)),
	bUseNullPropagation(FExpressionBuildHelper::GetUseNullPropagation(ExpressionObj))
{
}

bool FIndexExpression::IsNullPropagationEnabled() const
{
	if (bUseNullPropagation)
	{
		return true;
	}
		
	if (Expression.IsValid())
	{
		return Expression->IsNullPropagationEnabled();
	}
	return false;
}

FFormulaInvokeResult FIndexExpression::Execute(const FFormulaExecutionContext& Context) const
{
	if (!this->Expression.IsValid())
	{
		return FFormulaInvokeError::ExpressionIsInvalid();
	}
	
	const auto Result = this->Expression->Execute(Context);
	if (Result.HasError())
	{
		return Result; // propagate error
	}

	const bool IsNullPropagation = IsNullPropagationEnabled() || Context.AutoNullPropagation;
	const auto Target = Result.GetValue();
	if (Target->IsNull())
	{
		if (IsNullPropagation)
		{
			return FFormulaValue::Null();
		}
		else
		{
			return FFormulaInvokeError::NullReference();
		}
	}

	TArray<TSharedPtr<FFormulaValue>> PreparedArguments;
	for (const auto ArgumentPair : Arguments)
	{
		const auto ArgumentResult = ArgumentPair.Value->Execute(Context);
		if (ArgumentResult.HasError())
		{
			return ArgumentResult; // propagate error
		}
	}
	
	const FProperty* TargetType = Target->GetType();
	int32 ArrayIndex = 0;
	if (const FArrayProperty* ArrayProp = CastField<FArrayProperty>(TargetType);
		PreparedArguments.Num() == 1 && PreparedArguments[0]->TryGetInt32(ArrayIndex))
	{
		void* ArrayAddress;
		check(Target->TryGetContainerAddress(ArrayAddress));
		check(ArrayAddress != nullptr);
		
		FScriptArrayHelper ArrayWrap(ArrayProp, ArrayAddress);
		if (ArrayIndex < 0 || ArrayIndex >= ArrayWrap.Num())
		{
			return FFormulaInvokeError::IndexOutOfRange(ArrayIndex, ArrayWrap.Num());
		}
		const void* ElementPtr = ArrayWrap.GetElementPtr(ArrayIndex);
		check(ElementPtr != nullptr);

		return MakeShared<FFormulaValue>(ArrayProp->Inner, ElementPtr);
	}
	else if (const FSetProperty* SetProp = CastField<FSetProperty>(TargetType);
		PreparedArguments.Num() == 1  && PreparedArguments[0]->TryGetInt32(ArrayIndex))
	{
		void* SetAddress;
		check(Target->TryGetContainerAddress(SetAddress));
		check(SetAddress != nullptr);

		FScriptSetHelper SetWrap(SetProp, SetAddress);
		if (ArrayIndex < 0 || ArrayIndex >= SetWrap.Num())
		{
			return FFormulaInvokeError::IndexOutOfRange(ArrayIndex, SetWrap.Num());
		}
		const void* ElementPtr = SetWrap.GetElementPtr(ArrayIndex);
		check(ElementPtr != nullptr);

		return MakeShared<FFormulaValue>(ArrayProp->Inner, ElementPtr);
	}
	else if (const FMapProperty* MapProp = CastField<FMapProperty>(TargetType);
		PreparedArguments.Num() == 1)
	{
		void* MapAddress;
		check(Target->TryGetContainerAddress(MapAddress));
		check(MapAddress != nullptr);

		FScriptMapHelper MapWrap(MapProp,MapAddress);

		const TSharedPtr<FFormulaValue>& KeyArgument = PreparedArguments[0];
		TArray<uint8> ConvertedKeyValue;
		FProperty* KeyProperty = MapWrap.GetKeyProperty();
#if UE_VERSION_NEWER_THAN(5, 5, -1)
		ConvertedKeyValue.SetNumZeroed(KeyProperty->GetElementSize());
#else
		ConvertedKeyValue.SetNumZeroed(KeyProperty->ElementSize);
#endif
		if (KeyArgument->TryCopyCompleteValue(KeyProperty, ConvertedKeyValue.GetData()))
		{
			return FFormulaInvokeError::InvalidDictionaryKeyType(KeyArgument->ToString(), KeyArgument->GetType()->GetCPPType(), KeyProperty->GetCPPType());
		}

		const int32 MaxIndex = MapWrap.GetMaxIndex();
		for (int32 i = 0; i < MaxIndex; ++i)
		{
			if (!MapWrap.IsValidIndex(i))
			{
				continue;
			}

			const void* KeyPtr = MapWrap.GetKeyPtr(i);
			if (!KeyProperty->Identical(ConvertedKeyValue.GetData(), KeyPtr))
			{
				continue;
			}
			
			const void* ValuePtr = MapWrap.GetValuePtr(i);
			FProperty* ValueProperty = MapWrap.GetValueProperty();
			return MakeShared<FFormulaValue>(ValueProperty, ValuePtr);
		}
		
		return FFormulaInvokeError::DictionaryKeyNotFound(KeyArgument->ToString(), KeyArgument->GetType()->GetCPPType());
	}

	TArray<FString> ArgumentValueTypes;
	for (auto PreparedArgument : PreparedArguments)
	{
		ArgumentValueTypes.Push(PreparedArgument->GetType()->GetCPPType());
	}
	return FFormulaInvokeError::CantFindIndexer(TargetType->GetCPPType(), FString::Join(ArgumentValueTypes, TEXT(", ")));
}
