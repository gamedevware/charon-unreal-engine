// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "FFormulaValue.h"
#include "UObject/ObjectMacros.h"

class FFormulaExecutionContext;
class FFormulaExpression;

class CHARON_API FFormulaInvokeArguments
{
public:
	struct InvokeArgument
	{
		FString const Name;
		TSharedRef<FFormulaValue> const Value;
		EPropertyFlags const Flags;
	};
private:
	TMap<FString, InvokeArgument> ArgumentsByName;
	
public:
	FFormulaInvokeArguments();
	FFormulaInvokeArguments(std::initializer_list<InvokeArgument> InitList);

	const InvokeArgument* FindArgument(const FProperty* InParameter, int32 InParameterIndex, FString& OutParameterName) const;

	int Num() const;
	
	void InsertArgumentAt(const int32 Index, const int MaxParams, const TSharedRef<FFormulaValue>& InValue, EPropertyFlags Flags = EPropertyFlags::CPF_None);
	void AddArgument(const FString& InParameterName, const TSharedRef<FFormulaValue>& InValue, EPropertyFlags Flags = EPropertyFlags::CPF_None);
	void ReplaceArgumentValue(const FString& InParameterName, const TSharedRef<FFormulaValue>& InValue);
	void GetParameterTypes(TArray<FString>& OutParameterTypes);

	static EPropertyFlags GetArgumentFlags(const TSharedPtr<FFormulaExpression>& Expression, const TSharedRef<FFormulaValue>& Value, const FFormulaExecutionContext& Context);
};
