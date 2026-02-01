// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "FFormulaExecutionError.h"
#include "FFormulaValue.h"
#include "Misc/DateTime.h"
#include "Misc/Timespan.h"
#include "Misc/TVariant.h"
#include "Templates/SharedPointer.h"
#include "UObject/NameTypes.h"
#include "UObject/Object.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFormulaInvokeResult, Log, All);

class CHARON_API FFormulaExecutionResult
{
	TVariant<TSharedPtr<FFormulaValue>, FFormulaExecutionError> VariantValue;

public:
	// ReSharper disable CppNonExplicitConvertingConstructor
	FFormulaExecutionResult(const FFormulaExecutionError& Error) { VariantValue.Set<FFormulaExecutionError>(Error); }
	FFormulaExecutionResult(const TSharedPtr<FFormulaValue>& Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(Value); }
	FFormulaExecutionResult(const TSharedRef<FFormulaValue>& Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(Value); }
	FFormulaExecutionResult(bool bValue) { VariantValue.Set<TSharedPtr<FFormulaValue>>(bValue ? FFormulaValue::TrueBool() : FFormulaValue::FalseBool()); }
	FFormulaExecutionResult(nullptr_t) { VariantValue.Set<TSharedPtr<FFormulaValue>>(FFormulaValue::Null()); }
	FFormulaExecutionResult(UObject* Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(Value ? MakeShared<FFormulaValue>(Value) : FFormulaValue::Null()); }
	FFormulaExecutionResult(int8 Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(MakeShared<FFormulaValue>(Value)); }
	FFormulaExecutionResult(int16 Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(MakeShared<FFormulaValue>(Value)); }
	FFormulaExecutionResult(int32 Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(MakeShared<FFormulaValue>(Value)); }
	FFormulaExecutionResult(int64 Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(MakeShared<FFormulaValue>(Value)); }
	FFormulaExecutionResult(uint8 Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(MakeShared<FFormulaValue>(Value)); }
	FFormulaExecutionResult(uint32 Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(MakeShared<FFormulaValue>(Value)); }
	FFormulaExecutionResult(uint64 Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(MakeShared<FFormulaValue>(Value)); }
	FFormulaExecutionResult(float Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(MakeShared<FFormulaValue>(Value)); }
	FFormulaExecutionResult(double Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(MakeShared<FFormulaValue>(Value)); }
	FFormulaExecutionResult(FTimespan Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(MakeShared<FFormulaValue>(Value)); }
	FFormulaExecutionResult(FDateTime Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(MakeShared<FFormulaValue>(Value)); }
	FFormulaExecutionResult(FString Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(MakeShared<FFormulaValue>(Value)); }
	FFormulaExecutionResult(FName Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(MakeShared<FFormulaValue>(Value)); }
	FFormulaExecutionResult(FText Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(MakeShared<FFormulaValue>(Value)); }
	// ReSharper restore CppNonExplicitConvertingConstructor

	TSharedRef<FFormulaValue> GetValue() const
	{
		if (VariantValue.IsType<TSharedPtr<FFormulaValue>>())
		{
			const TSharedPtr<FFormulaValue> ValuePtr = VariantValue.Get<TSharedPtr<FFormulaValue>>();
			if (ValuePtr.IsValid())
			{
				return ValuePtr.ToSharedRef();
			}
			
			UE_LOG(LogTemp, Error, TEXT("FormulaInvokeResult stores a null pointer to an FFormulaValue instance. This can only happen if there's an error in the formula expression code."));
			return FFormulaValue::Null();
		}

		UE_LOG(LogTemp, Error, TEXT("FormulaInvokeResult stores an error message, not FFormulaValue, but a value was requested. This can only happen if there's an error in the formula expression code."));
		return FFormulaValue::Null();
	}
	bool HasError() const
	{
		return this->VariantValue.IsType<FFormulaExecutionError>();
	}
	const FFormulaExecutionError& GetError() const
	{
		return this->VariantValue.Get<FFormulaExecutionError>();
	}
};