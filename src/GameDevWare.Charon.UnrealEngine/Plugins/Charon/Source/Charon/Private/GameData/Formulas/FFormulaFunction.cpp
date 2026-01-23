#include "GameData/Formulas/FFormulaFunction.h"
#include "GameData/Formulas/IFormulaTypeDescription.h"

DEFINE_LOG_CATEGORY(LogFormulaMemberGroup);

bool IsAssignable(const IFormulaTypeDescription* BaseType, FProperty* DerivedType)
{
	if (const FObjectPropertyBase* ObjectDerivedType = CastField<FObjectPropertyBase>(DerivedType))
	{
		return BaseType->IsAssignableFrom(ObjectDerivedType->PropertyClass);
	}

	const auto BaseTypeCode = BaseType->GetTypeCode();
	const auto DerivedTypeCode = GetPropertyTypeCode(DerivedType);
	return BaseTypeCode == DerivedTypeCode &&
		BaseTypeCode != EFormulaValueType::ObjectPtr &&
		BaseTypeCode != EFormulaValueType::Struct;
}

bool IsAssignable(const FProperty* BaseType, const FProperty* DerivedType)
{
	// TODO unwrap optional types?

	if (const FObjectPropertyBase* ObjectBaseType = CastField<FObjectPropertyBase>(BaseType);
		const FObjectPropertyBase* ObjectDerivedType = CastField<FObjectPropertyBase>(DerivedType))
	{
		return ObjectDerivedType->PropertyClass->IsChildOf(ObjectBaseType->PropertyClass);
	}

	// TODO better argument binding like
	// constant narrowing or string lineral to UE types conversion
	// or numeric promotion
	return BaseType->SameType(DerivedType);
}

FFormulaFunction::FFormulaFunction(UFunction* Function, UClass* DeclaringClass, const bool bUseClassDefaultObject):
	FunctionInvoker(CreateDefaultFunctionInvoker(TWeakObjectPtr(Function), TWeakObjectPtr(DeclaringClass))),
	DeclaringTypePtr(TWeakObjectPtr(DeclaringClass)),
	bUseClassDefaultObject(bUseClassDefaultObject)
{
	check(Function);
	check(DeclaringClass);
}

FFormulaFunction::FFormulaFunction(FFormulaFunctionInvokeFunc FunctionInvoker, UField* DeclaringType,	const bool bUseClassDefaultObject):
	FunctionInvoker(MoveTemp(FunctionInvoker)),
	DeclaringTypePtr(TWeakObjectPtr(DeclaringType)),
	bUseClassDefaultObject(bUseClassDefaultObject)
{
	check(DeclaringType);
}

bool FFormulaFunction::TryInvoke(const TSharedRef<FFormulaValue>& Target,
                                 const TMap<FString, TSharedRef<FFormulaValue>>& CallArguments,
                                 const IFormulaTypeDescription* ExpectedType, const TArray<UField*>& TypeArguments,
                                 TSharedPtr<FFormulaValue>& Result) const
{
	TSharedRef<FFormulaValue> TargetOrDefault = Target;
	UClass* DeclaringClass = Cast<UClass>(this->DeclaringTypePtr.Get());
	if (DeclaringClass && bUseClassDefaultObject)
	{
		TargetOrDefault = MakeShared<FFormulaValue>(DeclaringClass->GetDefaultObject(/*bCreateIfNeeded*/ true));
	}
	return this->FunctionInvoker.IsSet() && this->FunctionInvoker(TargetOrDefault, CallArguments, ExpectedType, TypeArguments, Result);
}

FFormulaFunctionInvokeFunc FFormulaFunction::CreateDefaultFunctionInvoker(TWeakObjectPtr<UFunction> FunctionPtr, TWeakObjectPtr<UField> DeclaringClassPtr)
{
	return FFormulaFunctionInvokeFunc([FunctionPtr, DeclaringClassPtr](const TSharedRef<FFormulaValue>& Target,
								 const TMap<FString, TSharedRef<FFormulaValue>>& CallArguments,
								 const IFormulaTypeDescription* ExpectedType, const TArray<UField*>& TypeArguments,
								 TSharedPtr<FFormulaValue>& Result)
	{
		
		if (TypeArguments.Num() != 0)
		{
			return false; // type arguments are not supported
		}

		UFunction* Function = FunctionPtr.Get();
		UClass* DeclaringClass = Cast<UClass>(DeclaringClassPtr.Get());
		check(Function);
		check(DeclaringClass);
		
		if (!Function || !DeclaringClass)
		{
			return false; // function is gone
		}

		UObject* TargetPtr = nullptr;
		if (Target->GetTypeCode() == EFormulaValueType::ObjectPtr &&
			Target->TryCopyCompleteValue(Target->GetType(), &TargetPtr) &&
			!TargetPtr->GetClass()->IsChildOf(DeclaringClass))
		{
			TargetPtr = nullptr; // invalid target class
		}
		
		if (!TargetPtr)
		{
			return false; // invalid or null instance 
		}

		FProperty* ReturnValueParameter = Function->GetReturnProperty();
		if (ReturnValueParameter &&
			ExpectedType &&
			!IsAssignable(ExpectedType, ReturnValueParameter))
		{
			return false; // return type doesn't match
		}

		bool bIsMatching = true;
		FString ArgumentName;
		int32 ParameterIndex = 0;
		TArray<uint8> ArgumentsBuffer;
		ArgumentsBuffer.AddZeroed(Function->ParmsSize);
		void* ArgumentBufferPtr = ArgumentsBuffer.GetData();
		
		for (TFieldIterator<FProperty> It(Function); It; ++It)
		{
			const FProperty* Argument = *It;
			if (Argument->HasAnyPropertyFlags(CPF_ReturnParm))
			{
				continue;
			}

			Argument->GetName(ArgumentName);
			const TSharedRef<FFormulaValue>* ArgumentValuePtr = CallArguments.Find(ArgumentName);
			if (!ArgumentValuePtr)
			{
				ArgumentValuePtr = CallArguments.Find(FString::FromInt(ParameterIndex));
			}

			if (!ArgumentValuePtr || !IsAssignable(Argument, (*ArgumentValuePtr)->GetType()))
			{
				bIsMatching = false; // missing parameter or invalid type
				break;
			}

			const auto ArgumentValue = *ArgumentValuePtr;
			void* ArgumentPtr = Argument->ContainerPtrToValuePtr<void>(ArgumentBufferPtr);
			if (!ArgumentValue->TryCopyCompleteValue(Argument, ArgumentPtr))
			{
				bIsMatching = false; // argument bind failed
				break;
			}
			ParameterIndex++;
		}

		if (!bIsMatching)
		{
			return false;
		}

		TargetPtr->ProcessEvent(Function, ArgumentBufferPtr);

		if (ReturnValueParameter)
		{
			const void* ValuePtr = ReturnValueParameter->ContainerPtrToValuePtr<void>(ArgumentBufferPtr);
			Result = MakeShared<FFormulaValue>(ReturnValueParameter, ValuePtr);
			return true;
		}

		return false;
	});
}
