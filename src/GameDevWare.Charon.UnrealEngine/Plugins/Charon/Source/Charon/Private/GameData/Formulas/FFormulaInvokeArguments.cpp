// Copyright GameDevWare, Denis Zykov 2025

#include "GameData/Formulas/FFormulaInvokeArguments.h"
#include "GameData/Formulas/FFormulaExecutionContext.h"
#include "GameData/Formulas/EFormulaExpressionType.h"
#include "GameData/Formulas/Expressions/FFormulaExpression.h"

FFormulaInvokeArguments::FFormulaInvokeArguments()
{
}

FFormulaInvokeArguments::FFormulaInvokeArguments(std::initializer_list<InvokeArgument> InitList)
{
	for (InvokeArgument Argument : InitList)
	{
		this->ArgumentsByName.Add(Argument.Name, Argument);
	}
}

const FFormulaInvokeArguments::InvokeArgument* FFormulaInvokeArguments::FindArgument(const FProperty* InParameter, int32 InParameterIndex, FString& OutParameterName) const
{
	OutParameterName.Reset();
	InParameter->GetName(OutParameterName);
	if (const InvokeArgument* FoundArgument = this->ArgumentsByName.Find(OutParameterName))
	{
		return FoundArgument;
	}
			
	OutParameterName.Reset();
	OutParameterName.AppendInt(InParameterIndex);
	return this->ArgumentsByName.Find(OutParameterName);
}

int FFormulaInvokeArguments::Num() const
{
	return this->ArgumentsByName.Num();
}

void FFormulaInvokeArguments::InsertArgumentAt(const int32 Index, const int MaxParams, const TSharedRef<FFormulaValue>& InValue,
                                               EPropertyFlags Flags)
{
	check(Index >= 0);
	check(MaxParams < 255);
	
	// shift numeric parameters by one, so 0 became 1, 1 -> 2, 2 - 3 ... etc 
	FString ParameterIndexStr; 
	FString NewIndexStr;
	for (int32 i = MaxParams; i >= Index; i--)
	{
		ParameterIndexStr.Reset(); // Reset keeps the internal memory buffer allocated
		ParameterIndexStr.AppendInt(i);

		const InvokeArgument* FoundArgument = this->ArgumentsByName.Find(ParameterIndexStr);
		if (FoundArgument && this->ArgumentsByName.Remove(ParameterIndexStr))
		{
			NewIndexStr.Reset();
			NewIndexStr.AppendInt(i + 1);
    
			// Insert the value back with the incremented key
			this->ArgumentsByName.Add(NewIndexStr, InvokeArgument(NewIndexStr, FoundArgument->Value, nullptr, FoundArgument->Flags));
		}
	}
	NewIndexStr.Reset(); 
	NewIndexStr.AppendInt(Index);
	this->ArgumentsByName.Add(ParameterIndexStr, InvokeArgument(NewIndexStr, InValue, nullptr, Flags)); 
}

void FFormulaInvokeArguments::AddArgument(const FString& InParameterName, const TSharedRef<FFormulaValue>& InValue,
	const EPropertyFlags Flags)
{
	this->ArgumentsByName.Add(InParameterName, InvokeArgument(InParameterName, InValue, nullptr, Flags)); 
}

void FFormulaInvokeArguments::UpdateArgumentValue(const FString& InParameterName, const TSharedRef<FFormulaValue>& InUpdatedValue)
{
	if (const InvokeArgument* FoundArgument = this->ArgumentsByName.Find(InParameterName))
	{
		this->ArgumentsByName.Add(InParameterName, InvokeArgument(FoundArgument->Name, FoundArgument->Value, InUpdatedValue, FoundArgument->Flags)); 
	}
}

void FFormulaInvokeArguments::GetParameterTypes(TArray<FString>& OutParameterTypes)
{
	for (auto ArgumentPair : this->ArgumentsByName)
	{
		OutParameterTypes.Add(ArgumentPair.Value.Value->GetCPPType());
	}
}

TArray<FUpdatedArgumentPair> FFormulaInvokeArguments::GetUpdatedOutArguments()
{
	TArray<FUpdatedArgumentPair> OutArguments;
	for (auto Argument : this->ArgumentsByName)
	{
		if ((Argument.Value.Flags & EPropertyFlags::CPF_OutParm) != 0 && 
			Argument.Value.UpdatedValue.IsValid())
		{
			OutArguments.Add(FUpdatedArgumentPair(Argument.Value.Value, Argument.Value.UpdatedValue.ToSharedRef()));
		}
	}
	return OutArguments;
}

EPropertyFlags FFormulaInvokeArguments::GetArgumentFlags(const TSharedPtr<FFormulaExpression>& Expression,const TSharedRef<FFormulaValue>& Value, const FFormulaExecutionContext& Context)
{
	EPropertyFlags Flags = EPropertyFlags::CPF_None;
	if (Expression->GetType() == EFormulaExpressionType::ConstantExpression)
	{
		Flags |= EPropertyFlags::CPF_ConstParm;
	}

	const FString* ContextArgumentName = Context.Arguments.FindKey(Value);
	if (ContextArgumentName && Value->GetType()->HasAnyPropertyFlags(EPropertyFlags::CPF_OutParm))
	{
		Flags |= EPropertyFlags::CPF_OutParm;
	}
	return Flags;
}
