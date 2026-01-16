#include "GameData/Formulas/FFormulaMethodGroup.h"

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

	Functions.Add(TWeakObjectPtr(Function));
}

bool FFormulaMethodGroup::TryInvoke(const FFormulaVariableValue& Target,
                                    const TMap<FString, FFormulaVariableValue>& CallArguments,
                                    const FProperty* ExpectedType, const TArray<UClass*>& TypeArguments,
                                    FFormulaVariableValue& Result)
{
	if (this->Functions.Num() == 0)
	{
		return false; // empty function list
	}
	if (TypeArguments.Num() != 0)
	{
		return false; // type arguments are not supported
	}

	UObject* TargetPtr = nullptr;
	if (!Target.TryCopyCompleteValue(Target.GetFieldType(), &TargetPtr))
	{
		return false;
	}

	for (auto WeakFunctionPtr : this->Functions)
	{
		UFunction* Function = WeakFunctionPtr.Get();
		if (!Function)
		{
			continue; // function is gone
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
				if (ExpectedType && !Argument->SameType(ExpectedType))
				{
					bIsMatching = false; // return type doesn't match
					break;
				}
				continue;
			}

			FString PropName = Argument->GetName();
			const FFormulaVariableValue* ArgumentValue = CallArguments.Find(PropName);
			if (!ArgumentValue)
			{
				ArgumentValue = CallArguments.Find(FString::FromInt(ParameterIndex));
			}
			if (!ArgumentValue)
			{
				bIsMatching = false; // missing parameter
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

		if (FProperty* ReturnProp = Function->GetReturnProperty())
		{
			const void* ValuePtr = ReturnProp->ContainerPtrToValuePtr<void>(ArgumentsBuffer.GetData());
			Result = FFormulaVariableValue(ReturnProp, ValuePtr);
			return true;
		}
	}

	return false;
}
