#include "GameData/Formulas/FFormulaMethodGroup.h"
#include "GameData/Formulas/IFormulaTypeDescription.h"

DEFINE_LOG_CATEGORY(LogFormulaMemberGroup);

FFormulaMethodGroup::FFormulaMethodGroup()
{
}

void FFormulaMethodGroup::AddFunction(UFunction* Function)
{
	if (!Function)
	{
		UE_LOG(LogFormulaMemberGroup, Error, TEXT("Null function pointer is passed to AddFunction() method."));
		return;
	}

	Functions.Add(TWeakObjectPtr<UFunction>(Function));
}

bool IsAssignable(const IFormulaTypeDescription* BaseType, FProperty* DerivedType)
{
	if (const FObjectProperty* ObjectDerivedType = CastField<FObjectProperty>(DerivedType))
	{
		return BaseType->IsAssignableFrom(ObjectDerivedType->PropertyClass);
	}

	const auto BaseTypeCode = BaseType->GetTypeCode();
	const auto DerivedTypeCode = FFormulaValue::GetPropertyTypeCode(DerivedType);
	return BaseTypeCode == DerivedTypeCode &&
		BaseTypeCode != EFormulaValueType::Optional &&
		BaseTypeCode != EFormulaValueType::ObjectPtr &&
		BaseTypeCode != EFormulaValueType::Struct;
}

bool IsAssignable(const FProperty* BaseType, FProperty* DerivedType)
{
	// unwrap optional types?
	
	if (const FObjectProperty* ObjectBaseType = CastField<FObjectProperty>(BaseType);
		const FObjectProperty* ObjectDerivedType = CastField<FObjectProperty>(DerivedType))
	{
		return ObjectDerivedType->PropertyClass->IsChildOf(ObjectBaseType->PropertyClass);
	}
	
	return BaseType->SameType(DerivedType);
}

bool FFormulaMethodGroup::TryInvoke(const FFormulaValue& Target,
                                    const TMap<FString, FFormulaValue>& CallArguments,
                                    const IFormulaTypeDescription* ExpectedType, const TArray<UClass*>& TypeArguments,
                                    FFormulaValue& Result)
{
	if (TypeArguments.Num() != 0)
	{
		return false; // type arguments are not supported
	}

	UObject* TargetPtr = nullptr;
	if (!Target.TryCopyCompleteValue(Target.GetType(), &TargetPtr))
	{
		return false; // null target, all functions are non-static methods and require target
	}

	for (auto WeakFunctionPtr : this->Functions)
	{
		UFunction* Function = WeakFunctionPtr.Get();
		if (!Function)
		{
			continue; // function is gone
		}

		FProperty* ReturnValueParameter = Function->GetReturnProperty();
		if (ReturnValueParameter &&
			ExpectedType &&
			!IsAssignable(ExpectedType, ReturnValueParameter))
		{
			continue; // return type doesn't match
		}

		bool bIsMatching = true;
		int32 ParameterIndex = 0;
		TArray<uint8> ArgumentsBuffer;
		ArgumentsBuffer.AddZeroed(Function->ParmsSize);

		for (TFieldIterator<FProperty> It(Function); It; ++It)
		{
			const FProperty* Argument = *It;
			if (Argument->HasAnyPropertyFlags(CPF_ReturnParm))
			{
				continue;
			}

			FString PropName = Argument->GetName();
			const FFormulaValue* ArgumentValue = CallArguments.Find(PropName);
			if (!ArgumentValue)
			{
				ArgumentValue = CallArguments.Find(FString::FromInt(ParameterIndex));
			}

			if (!ArgumentValue || !IsAssignable(Argument, ArgumentValue->GetType()))
			{
				bIsMatching = false; // missing parameter or invalid type
				break;
			}

			void* ArgumentPtr = Argument->ContainerPtrToValuePtr<void>(ArgumentsBuffer.GetData());
			if (!ArgumentValue->TryCopyCompleteValue(Argument, ArgumentPtr))
			{
				bIsMatching = false; // argument bind failed
				break;
			}
			ParameterIndex++;
		}

		if (!bIsMatching)
		{
			continue;
		}

		TargetPtr->ProcessEvent(Function, ArgumentsBuffer.GetData());

		if (ReturnValueParameter)
		{
			const void* ValuePtr = ReturnValueParameter->ContainerPtrToValuePtr<void>(ArgumentsBuffer.GetData());
			Result = FFormulaValue(ReturnValueParameter, ValuePtr);
			return true;
		}
	}

	return false;
}
