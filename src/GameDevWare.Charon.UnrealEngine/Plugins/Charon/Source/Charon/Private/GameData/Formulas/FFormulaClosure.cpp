// Copyright GameDevWare, Denis Zykov 2025

#include "GameData/Formulas/FFormulaClosure.h"
#include "GameData/Formulas/DotNetTypes/UDotNetFunc.h"
#include "UObject/UnrealType.h"

FFormulaClosure::FFormulaClosure(
	const TSharedPtr<FFormulaExpression>& Body,
	const TArray<FString>& ParameterNames,
	const FFormulaExecutionContext& DeclaringContext
) :
	Body(Body),
	ParameterNames(ParameterNames),
	bAutoNullPropagation(DeclaringContext.AutoNullPropagation),
	CapturedArguments(DeclaringContext.Arguments),
	CapturedGlobal(DeclaringContext.Global),
	CapturedTypeResolver(DeclaringContext.TypeResolver)
{
}

FFormulaExecutionResult FFormulaClosure::Invoke(TArrayView<const TSharedRef<FFormulaValue>> Arguments) const
{
	if (!this->Body.IsValid())
	{
		return FFormulaExecutionError::ExpressionIsInvalid();
	}

	if (Arguments.Num() < this->ParameterNames.Num())
	{
		return FFormulaExecutionError::UnableToBindMethodToParameters(
			TEXT("Lambda"),
			TEXT("Invoke"),
			FString::Join(this->ParameterNames, TEXT(", ")),
			Arguments.Num()
		);
	}

	// parameters shadow the captured scope, so they are applied over a copy of it
	TMap<FString, const TSharedRef<FFormulaValue>> ScopeArguments = this->CapturedArguments;
	for (int32 ParameterIndex = 0; ParameterIndex < this->ParameterNames.Num(); ParameterIndex++)
	{
		ScopeArguments.Add(this->ParameterNames[ParameterIndex], Arguments[ParameterIndex]);
	}

	const FFormulaExecutionContext InvokeContext(
		this->bAutoNullPropagation,
		ScopeArguments,
		this->CapturedGlobal,
		this->CapturedTypeResolver
	);

	return this->Body->Execute(InvokeContext, nullptr);
}

TSharedRef<FFormulaValue> FFormulaClosure::ToValue(const TSharedRef<FFormulaClosure>& Closure)
{
	FDotNetFuncValue FuncValue;
	FuncValue.Closure = Closure;

	return MakeShared<FFormulaValue>(static_cast<FProperty*>(UDotNetFunc::GetLiteralProperty()), &FuncValue);
}

bool FFormulaClosure::TryGetFrom(const TSharedRef<FFormulaValue>& Value, TSharedPtr<FFormulaClosure>& OutClosure)
{
	const FStructProperty* StructProperty = CastField<FStructProperty>(Value->GetType());
	if (!StructProperty || StructProperty->Struct != FDotNetFuncValue::StaticStruct())
	{
		return false;
	}

	void* FuncValuePtr = nullptr;
	if (!Value->TryGetContainerAddress(FuncValuePtr) || !FuncValuePtr)
	{
		return false;
	}

	OutClosure = static_cast<FDotNetFuncValue*>(FuncValuePtr)->Closure;
	return OutClosure.IsValid();
}
