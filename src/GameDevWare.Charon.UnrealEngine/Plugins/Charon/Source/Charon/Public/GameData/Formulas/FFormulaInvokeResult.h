#pragma once
#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "FFormulaInvokeError.h"
#include "FFormulaValue.h"
#include "Misc/DateTime.h"
#include "Misc/Timespan.h"
#include "Misc/TVariant.h"
#include "Templates/SharedPointer.h"
#include "UObject/NameTypes.h"
#include "UObject/Object.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFormulaInvokeResult, Log, All);

class FFormulaInvokeResult
{
	TVariant<TSharedPtr<FFormulaValue>, FFormulaInvokeError> VariantValue;

public:
	FFormulaInvokeResult(const FFormulaInvokeError& Error) { VariantValue.Set<FFormulaInvokeError>(Error); }
	FFormulaInvokeResult(const TSharedPtr<FFormulaValue>& Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(Value); }
	FFormulaInvokeResult(const TSharedRef<FFormulaValue>& Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(Value); }
	FFormulaInvokeResult(bool bValue) { VariantValue.Set<TSharedPtr<FFormulaValue>>(bValue ? FFormulaValue::TrueBool() : FFormulaValue::FalseBool()); }
	FFormulaInvokeResult(nullptr_t) { VariantValue.Set<TSharedPtr<FFormulaValue>>(FFormulaValue::Null()); }
	FFormulaInvokeResult(UObject* Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(Value ? MakeShared<FFormulaValue>(Value) : FFormulaValue::Null()); }
	FFormulaInvokeResult(int8 Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(MakeShared<FFormulaValue>(Value)); }
	FFormulaInvokeResult(int16 Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(MakeShared<FFormulaValue>(Value)); }
	FFormulaInvokeResult(int32 Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(MakeShared<FFormulaValue>(Value)); }
	FFormulaInvokeResult(int64 Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(MakeShared<FFormulaValue>(Value)); }
	FFormulaInvokeResult(uint8 Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(MakeShared<FFormulaValue>(Value)); }
	FFormulaInvokeResult(uint32 Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(MakeShared<FFormulaValue>(Value)); }
	FFormulaInvokeResult(uint64 Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(MakeShared<FFormulaValue>(Value)); }
	FFormulaInvokeResult(float Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(MakeShared<FFormulaValue>(Value)); }
	FFormulaInvokeResult(double Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(MakeShared<FFormulaValue>(Value)); }
	FFormulaInvokeResult(FTimespan Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(MakeShared<FFormulaValue>(Value)); }
	FFormulaInvokeResult(FDateTime Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(MakeShared<FFormulaValue>(Value)); }
	FFormulaInvokeResult(FString Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(MakeShared<FFormulaValue>(Value)); }
	FFormulaInvokeResult(FName Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(MakeShared<FFormulaValue>(Value)); }
	FFormulaInvokeResult(FText Value) { VariantValue.Set<TSharedPtr<FFormulaValue>>(MakeShared<FFormulaValue>(Value)); }

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
		return this->VariantValue.IsType<FFormulaInvokeError>();
	}
	const FFormulaInvokeError& GetError() const
	{
		return this->VariantValue.Get<FFormulaInvokeError>();
	}
};