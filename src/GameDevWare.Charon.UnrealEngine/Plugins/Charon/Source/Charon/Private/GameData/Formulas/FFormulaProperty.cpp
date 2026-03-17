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
	GetterFunc(CreateDefaultPropertyGetter(TWeakFieldPtr<FProperty>(Property), TWeakObjectPtr<UField>(DeclaringType))),
	SetterFunc(CreateDefaultPropertySetter(TWeakFieldPtr<FProperty>(Property), TWeakObjectPtr<UField>(DeclaringType))),
	PropertyPtr(Property)
{
	check(Property);
	check(DeclaringType);
}

FFormulaProperty::FFormulaProperty(FProperty* Property, FFormulaPropertyGetterFunc GetterFunc, FFormulaPropertySetterFunc SetterFunc, UField* DeclaringType, const bool bUseClassDefaultObject):
	DeclaringTypePtr(DeclaringType),
	bUseClassDefaultObject(bUseClassDefaultObject),
	GetterFunc(MoveTemp(GetterFunc)),
	SetterFunc(MoveTemp(SetterFunc)),
	PropertyPtr(Property)
{
}

bool FFormulaProperty::TryGetValue(const TSharedRef<FFormulaValue>& InTarget, TSharedPtr<FFormulaValue>& OutValue) const
{
	TSharedRef<FFormulaValue> TargetOrDefault = InTarget;
	if (const UClass* DeclaringClass = Cast<UClass>(DeclaringTypePtr.Get()); DeclaringClass && bUseClassDefaultObject)
	{
		TargetOrDefault = MakeShared<FFormulaValue>(DeclaringClass->GetDefaultObject(/*bCreateIfNeeded*/ true));
	}
	return this->GetterFunc && this->GetterFunc(TargetOrDefault, OutValue);
}

bool FFormulaProperty::TrySetValue(const TSharedRef<FFormulaValue>& InTarget, const TSharedPtr<FFormulaValue>& InValue) const
{
	TSharedRef<FFormulaValue> TargetOrDefaultObject = InTarget;
	if (UClass* DeclaringClass = Cast<UClass>(DeclaringTypePtr.Get()); bUseClassDefaultObject)
	{
		TargetOrDefaultObject = MakeShared<FFormulaValue>(DeclaringClass->GetDefaultObject(/*bCreateIfNeeded*/ true));
	}
	return this->SetterFunc && this->SetterFunc(TargetOrDefaultObject, InValue);
}

FFormulaPropertyGetterFunc FFormulaProperty::CreateGetterFromFunctionInvoker(FFormulaFunctionInvokeFunc FunctionInvoker)
{
	return FFormulaPropertyGetterFunc([FunctionInvoker](const TSharedRef<FFormulaValue>& Target, TSharedPtr<FFormulaValue>& Result) -> bool
	{
		FFormulaInvokeArguments OperationArguments;
		return FunctionInvoker && FunctionInvoker(Target, OperationArguments, nullptr, nullptr, Result);
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
		return FunctionInvoker && FunctionInvoker(Target, OperationArguments, nullptr, nullptr, Result);
	});
}


FFormulaPropertyGetterFunc FFormulaProperty::CreateDefaultPropertyGetter(TWeakFieldPtr<FProperty> PropertyPtr, TWeakObjectPtr<UField> DeclaringTypePtr)
{
	return FFormulaPropertyGetterFunc([PropertyPtr, DeclaringTypePtr](const TSharedRef<FFormulaValue>& InTarget, TSharedPtr<FFormulaValue>& OutValue) -> bool
	{
		FProperty* PropertyOrNull = PropertyPtr.Get();
		UField* DeclaringTypeOrNull = DeclaringTypePtr.Get();
		check(PropertyOrNull);
		check(DeclaringTypeOrNull);
		
		if (!PropertyOrNull || !DeclaringTypeOrNull)
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
				UE_LOG(LogFormulaProperty, Warning, TEXT("Property get access failed because the call target is of the wrong type."));
				
				ContainerPtr = nullptr; // invalid target class
			}
			
			if (!ContainerPtr)
			{
				UE_LOG(LogFormulaProperty, Warning, TEXT("Property get access failed because the call target is null."));
				
				return false; // null reference target
			}
			
			const void* ValuePtr = PropertyOrNull->ContainerPtrToValuePtr<void>(ContainerPtr);
			OutValue = MakeShared<FFormulaValue>(PropertyOrNull, ValuePtr);
			return true;
		}
		else if (const UScriptStruct* DeclaringStruct = Cast<UScriptStruct>(DeclaringTypePtr.Get()))
		{
			void* ContainerPtr = nullptr;
			if (InTarget->GetTypeCode() == EFormulaValueType::Struct &&
				InTarget->TryGetContainerAddress(ContainerPtr) &&
				!GetScriptStruct(InTarget->GetType())->IsChildOf(DeclaringStruct))
			{
				UE_LOG(LogFormulaProperty, Warning, TEXT("Property get access failed because the call target is of the wrong type."));
				
				ContainerPtr = nullptr; // invalid target class
			}
			
			if (!ContainerPtr)
			{
				UE_LOG(LogFormulaProperty, Warning, TEXT("Property get access failed because the call target is null."));
				
				return false; // null reference target
			}

			const void* ValuePtr = PropertyOrNull->ContainerPtrToValuePtr<void>(ContainerPtr);
			OutValue = MakeShared<FFormulaValue>(PropertyOrNull, ValuePtr);
			return true;
		}
		return false;
	});
}

FFormulaPropertySetterFunc FFormulaProperty::CreateDefaultPropertySetter(TWeakFieldPtr<FProperty> PropertyPtr, TWeakObjectPtr<UField> DeclaringTypePtr)
{
	return FFormulaPropertySetterFunc([PropertyPtr, DeclaringTypePtr](const TSharedRef<FFormulaValue>& InTarget, const TSharedPtr<FFormulaValue>& InValue) -> bool
	{
		FProperty* PropertyOrNull = PropertyPtr.Get();
		UField* DeclaringTypeOrNull = DeclaringTypePtr.Get();
		check(PropertyOrNull);
		check(DeclaringTypeOrNull);
		
		if (!PropertyOrNull || !DeclaringTypeOrNull)
		{
			UE_LOG(LogFormulaProperty, Warning, TEXT("Property set access failed because the metadata object was unloaded or garbage collected."));

			return false; // metadata is gone
		}

		if (const UClass* DeclaringClass = Cast<UClass>(DeclaringTypeOrNull))
		{
			UObject* ContainerPtr = nullptr;
			if (InTarget->TryGetObjectPtr(ContainerPtr) &&
				!ContainerPtr->GetClass()->IsChildOf(DeclaringClass))
			{
				UE_LOG(LogFormulaProperty, Warning, TEXT("Property set access failed because the call target is of the wrong type."));
				
				ContainerPtr = nullptr; // invalid target class
			}
		
			if (!ContainerPtr)
			{
				UE_LOG(LogFormulaProperty, Warning, TEXT("Property set access failed because the call target is null."));
				
				return false; // null reference target
			}
			return InValue->TrySetPropertyValue_InContainer(PropertyOrNull, ContainerPtr, 0);
		}
		else if (const UScriptStruct* DeclaringStruct = Cast<UScriptStruct>(DeclaringTypeOrNull))
		{
			void* ContainerPtr = nullptr;
			if (InTarget->GetTypeCode() == EFormulaValueType::Struct &&
				InTarget->TryGetContainerAddress(ContainerPtr) &&
				!GetScriptStruct(InTarget->GetType())->IsChildOf(DeclaringStruct))
			{
				UE_LOG(LogFormulaProperty, Warning, TEXT("Property set access failed because the call target is of the wrong type."));
				
				ContainerPtr = nullptr; // invalid target class
			}
		
			if (!ContainerPtr)
			{
				UE_LOG(LogFormulaProperty, Warning, TEXT("Property set access failed because the call target is null."));
				
				return false; // null reference target
			}
		
			auto bSetSuccess = InValue->TrySetPropertyValue_InContainer(PropertyOrNull, ContainerPtr, 0);
			if (!bSetSuccess)
			{
				UE_LOG(LogFormulaProperty, Warning, TEXT("Property set access failed because the '%s' value of the %s property could not be cast/coerced to the %s type."), *InValue->GetCPPType(), *PropertyOrNull->GetName(), *FFormulaValue::GetExtendedCppName(PropertyOrNull) );
			}
			return bSetSuccess;
		}
		return false;
	});
}
