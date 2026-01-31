// Copyright GameDevWare, Denis Zykov 2025

#include "GameData/Formulas/FFormulaFunction.h"

#include "GameData/Formulas/FFormulaInvokeArguments.h"
#include "GameData/Formulas/IFormulaType.h"

DEFINE_LOG_CATEGORY(LogFormulaFunction);



FFormulaFunction::FFormulaFunction(UFunction* Function, UClass* DeclaringClass, const bool bUseClassDefaultObject) :
	FunctionInvoker(CreateDefaultFunctionInvoker(Function, DeclaringClass)),
	DeclaringTypePtr(TWeakObjectPtr(DeclaringClass)),
	bUseClassDefaultObject(bUseClassDefaultObject),
	Parameters(GetFunctionParameters(Function))
{
	check(Function);
	check(DeclaringClass);
}

FFormulaFunction::FFormulaFunction(
	FFormulaFunctionInvokeFunc FunctionInvoker,
	const TArray<const FProperty*>& Parameters,
	UField* DeclaringType,
	const bool bUseClassDefaultObject
) :
	FunctionInvoker(MoveTemp(FunctionInvoker)),
	DeclaringTypePtr(TWeakObjectPtr(DeclaringType)),
	bUseClassDefaultObject(bUseClassDefaultObject),
	Parameters(Parameters)
{
	check(DeclaringType);
}

bool FFormulaFunction::TryInvoke(
	const TSharedRef<FFormulaValue>& Target,
	FFormulaInvokeArguments& CallArguments,
	const UField* ExpectedType,
	const TArray<UField*>* TypeArguments,
	TSharedPtr<FFormulaValue>& Result) const
{
	const UClass* DeclaringClass = Cast<UClass>(this->DeclaringTypePtr.Get());
	check(DeclaringClass);

	TSharedRef<FFormulaValue> TargetOrDefault = Target;
	if (DeclaringClass && bUseClassDefaultObject)
	{
		TargetOrDefault = MakeShared<FFormulaValue>(DeclaringClass->GetDefaultObject(/*bCreateIfNeeded*/ true));
	}
	return this->FunctionInvoker.IsSet() && this->FunctionInvoker(TargetOrDefault, CallArguments, ExpectedType,
	                                                              TypeArguments, Result);
}

FFormulaFunctionInvokeFunc FFormulaFunction::CreateDefaultFunctionInvoker(UFunction* Function, UField* DeclaringClass)
{
	TWeakObjectPtr<UFunction> FunctionPtr = TWeakObjectPtr(Function);
	TWeakObjectPtr<UField> DeclaringClassPtr = TWeakObjectPtr(DeclaringClass);
	
	return FFormulaFunctionInvokeFunc([FunctionPtr, DeclaringClassPtr](
		const TSharedRef<FFormulaValue>& Target,
		FFormulaInvokeArguments& CallArguments,
		const UField* ExpectedType,
		const TArray<UField*>* TypeArguments,
		TSharedPtr<FFormulaValue>& Result)
	{
		if (TypeArguments && TypeArguments->Num() != 0)
		{
			UE_LOG(LogFormulaFunction, Warning, TEXT("Method binding failed because generic arguments were specified. By default, Blueprint functions do not support generic arguments."));
			return false; // type arguments are not supported
		}

		UFunction* Function = FunctionPtr.Get();
		const UClass* DeclaringClass = Cast<UClass>(DeclaringClassPtr.Get());
		check(Function);
		check(DeclaringClass);

		if (!Function || !DeclaringClass)
		{
			UE_LOG(LogFormulaFunction, Warning, TEXT("Method binding failed because the metadata object was unloaded or garbage collected."));
			return false; // function is gone
		}

		UObject* TargetPtr = nullptr;
		if (Target->TryGetObjectPtr(TargetPtr) &&
			!TargetPtr->GetClass()->IsChildOf(DeclaringClass))
		{
			TargetPtr = nullptr; // invalid target class
		}

		if (!TargetPtr)
		{
			UE_LOG(LogFormulaFunction, Warning, TEXT("Method binding failed because the call target is null or of the wrong type."));
			return false; // invalid or null instance 
		}

		/*
		if (ReturnValueParameter &&
			ExpectedType &&
			!IsAssignable(ExpectedType, ReturnValueParameter))
		{
			return false; // return type doesn't match
		}
		*/

		FString ParameterName;
		bool bIsMatching = true;
		
		TArray<uint8> ArgumentsBuffer;
		ArgumentsBuffer.AddZeroed(Function->ParmsSize);
		void* ArgumentBufferPtr = ArgumentsBuffer.GetData();

		// bind input parameters
		int32 ParameterIndex = 0;
		for (TFieldIterator<FProperty> It(Function); It; ++It, ParameterIndex++)
		{
			const FProperty* Parameter = *It;
			if (Parameter->HasAnyPropertyFlags(CPF_ReturnParm))
			{
				ParameterIndex--;
				continue;
			}

			const auto InvokeArgumentPtr = CallArguments.FindArgument(Parameter, ParameterIndex, ParameterName);
			if (!InvokeArgumentPtr)
			{
				UE_LOG(LogFormulaFunction, Warning, TEXT("Method binding failed because the required parameter %s[#%d] was not found in the argument list."), *Parameter->GetName(), ParameterIndex);
				bIsMatching = false;
				break;
			}
			
			void* ArgumentDataPtr = Parameter->ContainerPtrToValuePtr<void>(ArgumentBufferPtr);
			if (!InvokeArgumentPtr->Value->TryCopyCompleteValue(Parameter, ArgumentDataPtr))
			{
				UE_LOG(LogFormulaFunction, Warning, TEXT("Method binding failed because the '%s' value of the %s parameter could not be cast/coerced to the %s type."), *InvokeArgumentPtr->Value->GetCPPType(), *Parameter->GetName(), *FFormulaValue::GetExtendedCppName(Parameter) );
				bIsMatching = false; // argument bind failed
				break;
			}
		}

		if (!bIsMatching)
		{
			return false;
		}

		TargetPtr->ProcessEvent(Function, ArgumentBufferPtr);

		// copy output parameters
		ParameterIndex = 0;
		for (TFieldIterator<FProperty> It(Function); It; ++It, ++ParameterIndex)
		{
			FProperty* Parameter = *It;
			if (Parameter->HasAnyPropertyFlags(CPF_ReturnParm))
			{
				const void* ValuePtr = Parameter->ContainerPtrToValuePtr<void>(ArgumentBufferPtr);
				Result = MakeShared<FFormulaValue>(Parameter, ValuePtr);
				ParameterIndex--;
			}
			if (Parameter->HasAnyPropertyFlags(CPF_OutParm))
			{
				const auto ArgumentValuePtr = CallArguments.FindArgument(Parameter, ParameterIndex, ParameterName);
				if (!ArgumentValuePtr)
				{
					continue;
				}
				const void* ValuePtr = Parameter->ContainerPtrToValuePtr<void>(ArgumentBufferPtr);
				CallArguments.UpdateArgumentValue(ParameterName, MakeShared<FFormulaValue>(Parameter, ValuePtr));
			}
		}
		return true;
	});
}

FFormulaFunctionInvokeFunc FFormulaFunction::CreateExtensionFunctionInvoker(UFunction* Function, UField* DeclaringClass)
{
	TWeakObjectPtr<UFunction> FunctionPtr = TWeakObjectPtr(Function);
	TWeakObjectPtr<UField> DeclaringClassPtr = TWeakObjectPtr(DeclaringClass);
	FFormulaFunctionInvokeFunc DefaultFunctionInvoker = CreateDefaultFunctionInvoker(Function, DeclaringClass);
	return FFormulaFunctionInvokeFunc([DeclaringClassPtr, FunctionPtr, DefaultFunctionInvoker](
			const TSharedRef<FFormulaValue>& Target,
			FFormulaInvokeArguments& CallArguments,
			const UField* ExpectedType,
			const TArray<UField*>* TypeArguments,
			TSharedPtr<FFormulaValue>& Result)
		{
			const UClass* DeclaringClass = Cast<UClass>(DeclaringClassPtr.Get());
			const UFunction* Function = FunctionPtr.Get();
			check(DeclaringClass);
			check(Function && Function->HasAnyFunctionFlags(EFunctionFlags::FUNC_Static));

			if (!Function || !DeclaringClass)
			{
				return false; // function or class is gone
			}
		
			// replace Target with CDO because function is static
			const TSharedRef<FFormulaValue> TargetDefaultClassObject = MakeShared<FFormulaValue>(DeclaringClass->GetDefaultObject(/*bCreateIfNeeded*/ true));

			
			CallArguments.InsertArgumentAt(0, Function->NumParms, Target);
		
			return DefaultFunctionInvoker(TargetDefaultClassObject, CallArguments, ExpectedType, TypeArguments, Result);
		});
}

TArray<const FProperty*> FFormulaFunction::GetFunctionParameters(const UFunction* Function)
{
	check(Function);
	
	TArray<const FProperty*> Parameters;
	for (TFieldIterator<FProperty> It(Function); It; ++It)
	{
		const FProperty* Parameter = *It;
		if (Parameter->HasAnyPropertyFlags(CPF_ReturnParm))
		{
			continue;
		}
		Parameters.Add(Parameter);
	}
	return Parameters;
}
