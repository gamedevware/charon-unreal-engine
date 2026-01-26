// Copyright GameDevWare, Denis Zykov 2025

#include "GameData/Formulas/Expressions/FIndexExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"

FIndexExpression::FIndexExpression(const TSharedRef<FJsonObject>& ExpressionObj) :
	Arguments(FExpressionBuildHelper::GetArguments(ExpressionObj, FFormulaNotation::ARGUMENTS_ATTRIBUTE)),
	Expression(FExpressionBuildHelper::GetExpression(ExpressionObj, FFormulaNotation::EXPRESSION_ATTRIBUTE)),
	bUseNullPropagation(FExpressionBuildHelper::GetUseNullPropagation(ExpressionObj))
{
}

FIndexExpression::FIndexExpression(const TSharedPtr<FFormulaExpression>& Expression,
	const TMap<FString, TSharedPtr<FFormulaExpression>>& Arguments, const bool bUseNullPropagation) :
	Arguments(Arguments), Expression(Expression), bUseNullPropagation(bUseNullPropagation)
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

FFormulaExecutionResult FIndexExpression::Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const
{
	if (!this->IsValid())
	{
		return FFormulaExecutionError::ExpressionIsInvalid();
	}
	
	const auto Result = this->Expression->Execute(Context, nullptr);
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
			return FFormulaExecutionError::NullReference();
		}
	}

	const FProperty* TargetType = Target->GetType();
	bool bFirstArgument = true;
	TArray<TSharedPtr<FFormulaValue>> PreparedArguments;
	for (const auto ArgumentPair : this->Arguments)
	{
		FProperty* ExpectedIndexerType = nullptr;
		if (bFirstArgument)
		{
			if (CastField<FArrayProperty>(TargetType) || CastField<FSetProperty>(TargetType))
			{
				ExpectedIndexerType = UDotNetInt32::GetLiteralProperty();
			}
			else if (const FMapProperty* MapProp = CastField<FMapProperty>(TargetType))
			{
				ExpectedIndexerType = MapProp->KeyProp;
			}
		}
		
		const auto ArgumentResult = ArgumentPair.Value->Execute(Context, ExpectedIndexerType);
		if (ArgumentResult.HasError())
		{
			return ArgumentResult; // propagate error
		}
		PreparedArguments.Add(ArgumentResult.GetValue());
		bFirstArgument = false;
	}
	
	int32 ArrayIndex = 0;
	if (const FArrayProperty* ArrayProp = CastField<FArrayProperty>(TargetType);
		ArrayProp && PreparedArguments.Num() == 1 && PreparedArguments[0]->TryGetInt32(ArrayIndex))
	{
		void* ArrayAddress;
		check(Target->TryGetContainerAddress(ArrayAddress));
		check(ArrayAddress != nullptr);
		
		FScriptArrayHelper ArrayWrap(ArrayProp, ArrayAddress);
		if (ArrayIndex < 0 || ArrayIndex >= ArrayWrap.Num())
		{
			return FFormulaExecutionError::IndexOutOfRange(ArrayIndex, ArrayWrap.Num());
		}
		const void* ElementPtr = ArrayWrap.GetElementPtr(ArrayIndex);
		check(ElementPtr != nullptr);

		return MakeShared<FFormulaValue>(ArrayProp->Inner, ElementPtr);
	}
	else if (const FSetProperty* SetProp = CastField<FSetProperty>(TargetType);
		SetProp && PreparedArguments.Num() == 1  && PreparedArguments[0]->TryGetInt32(ArrayIndex))
	{
		void* SetAddress;
		check(Target->TryGetContainerAddress(SetAddress));
		check(SetAddress != nullptr);

		FScriptSetHelper SetWrap(SetProp, SetAddress);
		if (ArrayIndex < 0 || ArrayIndex >= SetWrap.Num())
		{
			return FFormulaExecutionError::IndexOutOfRange(ArrayIndex, SetWrap.Num());
		}
		const void* ElementPtr = SetWrap.GetElementPtr(ArrayIndex);
		check(ElementPtr != nullptr);

		return MakeShared<FFormulaValue>(SetProp->ElementProp, ElementPtr);
	}
	else if (const FMapProperty* MapProp = CastField<FMapProperty>(TargetType);
		MapProp && PreparedArguments.Num() == 1)
	{
		void* MapAddress;
		check(Target->TryGetContainerAddress(MapAddress));
		check(MapAddress != nullptr);

		FScriptMapHelper MapWrap(MapProp,MapAddress);

		const TSharedPtr<FFormulaValue>& KeyArgument = PreparedArguments[0];
		TArray<uint8> ConvertedKeyValue;
		FProperty* KeyProperty = MapWrap.GetKeyProperty();
		ConvertedKeyValue.SetNumZeroed(KeyProperty->GetSize());
		if (!KeyArgument->TryCopyCompleteValue(KeyProperty, ConvertedKeyValue.GetData()))
		{
			return FFormulaExecutionError::InvalidDictionaryKeyType(KeyArgument->ToString(), KeyArgument->GetCPPType(), KeyProperty->GetCPPType());
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
		
		return FFormulaExecutionError::DictionaryKeyNotFound(KeyArgument->ToString(), KeyArgument->GetCPPType());
	}

	TArray<FString> ArgumentValueTypes;
	for (auto PreparedArgument : PreparedArguments)
	{
		ArgumentValueTypes.Push(PreparedArgument->GetCPPType());
	}
	return FFormulaExecutionError::CantFindIndexer(TargetType->GetCPPType(), FString::Join(ArgumentValueTypes, TEXT(", ")));
}

bool FIndexExpression::IsValid() const
{
	if (!this->Expression.IsValid())
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

void FIndexExpression::DebugPrintTo(FString& OutValue) const
{
	if (this->Expression.IsValid())
	{
		if (this->Expression->GetType() != EFormulaExpressionType::MemberExpression)
		{
			OutValue.Append("(");
		}
		this->Expression->DebugPrintTo(OutValue);
		
		if (this->Expression->GetType() != EFormulaExpressionType::MemberExpression)
		{
			OutValue.Append(")");
		}
	}
	else
	{
		OutValue.Append(TEXT("#INVALID#"));
	}
	if (this->bUseNullPropagation)
	{
		OutValue.Append("?");
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
