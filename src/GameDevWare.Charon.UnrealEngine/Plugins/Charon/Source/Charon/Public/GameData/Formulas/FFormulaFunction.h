// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "FFormulaInvokeArguments.h"
#include "FFormulaValue.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Templates/SharedPointer.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/StrongObjectPtr.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFormulaFunction, Log, All);

class IFormulaType;

using FFormulaFunctionInvokeFunc = TFunction<bool(const TSharedRef<FFormulaValue>&, FFormulaInvokeArguments&, const UField*, const TArray<UField*>*, TSharedPtr<FFormulaValue>&)>;

class CHARON_API FFormulaFunction
{
private:
	const FFormulaFunctionInvokeFunc FunctionInvoker;
	const TWeakObjectPtr<UField> DeclaringTypePtr;
	const bool bUseClassDefaultObject;
	
public:
	FFormulaFunction(UFunction* Function, UClass* DeclaringClass, const bool bUseClassDefaultObject);
	FFormulaFunction(FFormulaFunctionInvokeFunc Function, UField* DeclaringType, const bool bUseClassDefaultObject);
	
	bool TryInvoke(
		const TSharedRef<FFormulaValue>& Target,
		FFormulaInvokeArguments& CallArguments,
		const UField* ExpectedType,
		const TArray<UField*>* TypeArguments,
		TSharedPtr<FFormulaValue>& Result
	) const;

	// Create basic function invoker, which will try to bind parameters to specified Function
	static FFormulaFunctionInvokeFunc CreateDefaultFunctionInvoker(UFunction* Function, UField* DeclaringClass);
	
	// Create invoker for extension function.
	// Extension function is static function where call target goes as first parameter and named 'Self' (eg. int32 MyFunc(USomeType Self, int32 Parameter), this
	// TFunction wrapper inserts Target into first position with name '0' and shift other parameters forward by one.
	static FFormulaFunctionInvokeFunc CreateExtensionFunctionInvoker(UFunction* Function, UField* DeclaringClass);
};
