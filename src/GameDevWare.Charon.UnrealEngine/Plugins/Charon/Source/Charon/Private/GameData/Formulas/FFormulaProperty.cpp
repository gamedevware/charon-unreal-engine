// Copyright GameDevWare, Denis Zykov 2025

#include "GameData/Formulas/FFormulaProperty.h"

DEFINE_LOG_CATEGORY(LogFormulaProperty);

static UScriptStruct* GetScriptStruct(const FProperty* Property)
{
	check(Property);
	
	if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
	{
		return StructProperty->Struct;
	}
	
	checkNoEntry();
	return nullptr;
}

FFormulaProperty::FFormulaProperty(FProperty* Property, UField* DeclaringType, const bool bUseClassDefaultObject):
	DeclaringTypePtr(DeclaringType),
	bUseClassDefaultObject(bUseClassDefaultObject),
	GetterFunc(CreateDefaultPropertyGetter(TWeakFieldPtr<FProperty>(Property), TWeakObjectPtr(DeclaringType))),
	SetterFunc(CreateDefaultPropertySetter(TWeakFieldPtr<FProperty>(Property), TWeakObjectPtr(DeclaringType))),
	Property(Property)
{
	check(Property);
	check(DeclaringType);
}

FFormulaProperty::FFormulaProperty(FProperty* Property, FFormulaPropertyGetterFunc GetterFunc, FFormulaPropertySetterFunc SetterFunc, UField* DeclaringType, const bool bUseClassDefaultObject):
	DeclaringTypePtr(DeclaringType),
	bUseClassDefaultObject(bUseClassDefaultObject),
	GetterFunc(MoveTemp(GetterFunc)),
	SetterFunc(MoveTemp(SetterFunc)),
	Property(Property)
{
}

bool FFormulaProperty::TryGetValue(const TSharedRef<FFormulaValue>& InTarget, TSharedPtr<FFormulaValue>& OutValue) const
{
	TSharedRef<FFormulaValue> TargetOrDefault = InTarget;
	if (UClass* DeclaringClass = Cast<UClass>(DeclaringTypePtr.Get()); bUseClassDefaultObject)
	{
		TargetOrDefault = MakeShared<FFormulaValue>(DeclaringClass->GetDefaultObject(/*bCreateIfNeeded*/ true));
	}
	return this->GetterFunc.IsSet() && this->GetterFunc(TargetOrDefault, OutValue);
}

bool FFormulaProperty::TrySetValue(const TSharedRef<FFormulaValue>& InTarget, const TSharedPtr<FFormulaValue>& InValue) const
{
	TSharedRef<FFormulaValue> TargetOrDefaultObject = InTarget;
	if (UClass* DeclaringClass = Cast<UClass>(DeclaringTypePtr.Get()); bUseClassDefaultObject)
	{
		TargetOrDefaultObject = MakeShared<FFormulaValue>(DeclaringClass->GetDefaultObject(/*bCreateIfNeeded*/ true));
	}
	return this->SetterFunc.IsSet() && this->SetterFunc(TargetOrDefaultObject, InValue);
}

FFormulaPropertyGetterFunc FFormulaProperty::CreateGetterFromFunctionInvoker(FFormulaFunctionInvokeFunc FunctionInvoker)
{
	return FFormulaPropertyGetterFunc([FunctionInvoker](const TSharedRef<FFormulaValue>& Target, TSharedPtr<FFormulaValue>& Result) -> bool
	{
		FFormulaInvokeArguments OperationArguments;
		return FunctionInvoker.IsSet() && FunctionInvoker(Target, OperationArguments, nullptr, nullptr, Result);
	});
}

FFormulaPropertySetterFunc FFormulaProperty::CreateSetterFromFunctionInvoker(FFormulaFunctionInvokeFunc FunctionInvoker)
{
	return FFormulaPropertySetterFunc([FunctionInvoker](const TSharedRef<FFormulaValue>& Target, const TSharedPtr<FFormulaValue>& Value) -> bool
	{
		FFormulaInvokeArguments OperationArguments {
			FFormulaInvokeArguments::InvokeArgument(TEXT("0"), Value.ToSharedRef(), nullptr, EPropertyFlags::CPF_None),
		};
		TSharedPtr<FFormulaValue> Result; // discarded
		return FunctionInvoker.IsSet() && FunctionInvoker(Target, OperationArguments, nullptr, nullptr, Result);
	});
}


FFormulaPropertyGetterFunc FFormulaProperty::CreateDefaultPropertyGetter(TWeakFieldPtr<FProperty> PropertyPtr, TWeakObjectPtr<UField> DeclaringTypePtr)
{
	return FFormulaPropertyGetterFunc([PropertyPtr, DeclaringTypePtr](const TSharedRef<FFormulaValue>& InTarget, TSharedPtr<FFormulaValue>& OutValue) -> bool
	{
		FProperty* Property = PropertyPtr.Get();
		UField* DeclaringType = DeclaringTypePtr.Get();
		check(Property);
		check(DeclaringType);
		
		if (!Property || !DeclaringType)
		{
			UE_LOG(LogFormulaProperty, Warning, TEXT("Property get access failed because the metadata object was unloaded or garbage collected."));
			
			return false;  // metadata is gone
		}

		if (const UClass* DeclaringClass = Cast<UClass>(DeclaringTypePtr.Get()))
		{
			UObject* ContainerPtr = nullptr;
			if (InTarget->TryGetObjectPtr(ContainerPtr) &&
				!ContainerPtr->GetClass()->IsChildOf(DeclaringClass))
			{
				UE_LOG(LogFormulaProperty, Warning, TEXT("Property get access failed because the call target of the wrong type."));
				
				ContainerPtr = nullptr; // invalid target class
			}
			
			if (!ContainerPtr)
			{
				UE_LOG(LogFormulaProperty, Warning, TEXT("Property get access failed because the call target is null."));
				
				return false; // null reference target
			}
			
			const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(ContainerPtr);
			OutValue = MakeShared<FFormulaValue>(Property, ValuePtr);
			return true;
		}
		else if (const UScriptStruct* DeclaringStruct = Cast<UScriptStruct>(DeclaringTypePtr.Get()))
		{
			void* ContainerPtr = nullptr;
			if (InTarget->GetTypeCode() == EFormulaValueType::Struct &&
				InTarget->TryGetContainerAddress(ContainerPtr) &&
				GetScriptStruct(InTarget->GetType())->IsChildOf(DeclaringStruct))
			{
				UE_LOG(LogFormulaProperty, Warning, TEXT("Property get access failed because the call target of the wrong type."));
				
				ContainerPtr = nullptr; // invalid target class
			}
			
			if (!ContainerPtr)
			{
				UE_LOG(LogFormulaProperty, Warning, TEXT("Property get access failed because the call target is null."));
				
				return false; // null reference target
			}

			const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(ContainerPtr);
			OutValue = MakeShared<FFormulaValue>(Property, ValuePtr);
			return true;
		}
		return false;
	});
}

FFormulaPropertySetterFunc FFormulaProperty::CreateDefaultPropertySetter(TWeakFieldPtr<FProperty> PropertyPtr, TWeakObjectPtr<UField> DeclaringTypePtr)
{
	return FFormulaPropertySetterFunc([PropertyPtr, DeclaringTypePtr](const TSharedRef<FFormulaValue>& InTarget, const TSharedPtr<FFormulaValue>& InValue) -> bool
	{
		FProperty* Property = PropertyPtr.Get();
		UField* DeclaringType = DeclaringTypePtr.Get();
		check(Property);
		check(DeclaringType);
		
		if (!Property || !DeclaringType)
		{
			UE_LOG(LogFormulaProperty, Warning, TEXT("Property set access failed because the metadata object was unloaded or garbage collected."));

			return false; // metadata is gone
		}

		if (const UClass* DeclaringClass = Cast<UClass>(DeclaringType))
		{
			UObject* ContainerPtr = nullptr;
			if (InTarget->TryGetObjectPtr(ContainerPtr) &&
				!ContainerPtr->GetClass()->IsChildOf(DeclaringClass))
			{
				UE_LOG(LogFormulaProperty, Warning, TEXT("Property set access failed because the call target of the wrong type."));
				
				ContainerPtr = nullptr; // invalid target class
			}
		
			if (!ContainerPtr)
			{
				UE_LOG(LogFormulaProperty, Warning, TEXT("Property set access failed because the call target is null."));
				
				return false; // null reference target
			}
			return InValue->TrySetPropertyValue_InContainer(Property, ContainerPtr, 0);
		}
		else if (const UScriptStruct* DeclaringStruct = Cast<UScriptStruct>(DeclaringType))
		{
			void* ContainerPtr = nullptr;
			if (InTarget->GetTypeCode() == EFormulaValueType::Struct &&
				InTarget->TryGetContainerAddress(ContainerPtr) &&
				GetScriptStruct(InTarget->GetType())->IsChildOf(DeclaringStruct))
			{
				UE_LOG(LogFormulaProperty, Warning, TEXT("Property set access failed because the call target of the wrong type."));
				
				ContainerPtr = nullptr; // invalid target class
			}
		
			if (!ContainerPtr)
			{
				UE_LOG(LogFormulaProperty, Warning, TEXT("Property set access failed because the call target is null."));
				
				return false; // null reference target
			}
		
			auto bSetSuccess = InValue->TrySetPropertyValue_InContainer(Property, ContainerPtr, 0);
			if (!bSetSuccess)
			{
				UE_LOG(LogFormulaProperty, Warning, TEXT("Property set access failed because the '%s' value of the %s value could not be cast/coerced to the %s type."), *InValue->GetCPPType(), *Property->GetName(), *FFormulaValue::GetExtendedCppName(Property) );
			}
			return bSetSuccess;
		}
		return false;
	});
}
