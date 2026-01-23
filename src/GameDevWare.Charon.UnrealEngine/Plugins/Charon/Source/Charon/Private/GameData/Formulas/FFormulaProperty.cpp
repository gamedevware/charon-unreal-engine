#include "GameData/Formulas/FFormulaProperty.h"

UScriptStruct* GetScriptStruct(const FProperty* Property)
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
	SetterFunc(CreateDefaultPropertySetter(TWeakFieldPtr<FProperty>(Property), TWeakObjectPtr(DeclaringType)))
{
	check(Property);
	check(DeclaringType);
}

FFormulaProperty::FFormulaProperty(FFormulaPropertyGetterFunc GetterFunc, FFormulaPropertySetterFunc SetterFunc, UField* DeclaringType, const bool bUseClassDefaultObject):
	DeclaringTypePtr(DeclaringType), bUseClassDefaultObject(bUseClassDefaultObject), GetterFunc(MoveTemp(GetterFunc)), SetterFunc(MoveTemp(SetterFunc))
{
}

bool FFormulaProperty::TryGetValue(const TSharedRef<FFormulaValue>& Target, TSharedPtr<FFormulaValue>& Result) const
{
	TSharedRef<FFormulaValue> TargetOrDefault = Target;
	if (UClass* DeclaringClass = Cast<UClass>(DeclaringTypePtr.Get()); bUseClassDefaultObject)
	{
		TargetOrDefault = MakeShared<FFormulaValue>(DeclaringClass->GetDefaultObject(/*bCreateIfNeeded*/ true));
	}
	return this->GetterFunc.IsSet() && this->GetterFunc(TargetOrDefault, Result);
}

bool FFormulaProperty::TrySetValue(const TSharedRef<FFormulaValue>& Target, const TSharedPtr<FFormulaValue>& Value) const
{
	TSharedRef<FFormulaValue> TargetOrDefaultObject = Target;
	if (UClass* DeclaringClass = Cast<UClass>(DeclaringTypePtr.Get()); bUseClassDefaultObject)
	{
		TargetOrDefaultObject = MakeShared<FFormulaValue>(DeclaringClass->GetDefaultObject(/*bCreateIfNeeded*/ true));
	}
	return this->SetterFunc.IsSet() && this->SetterFunc(TargetOrDefaultObject, Value);
}


FFormulaPropertyGetterFunc FFormulaProperty::CreateDefaultPropertyGetter(TWeakFieldPtr<FProperty> PropertyPtr, TWeakObjectPtr<UField> DeclaringTypePtr)
{
	return FFormulaPropertyGetterFunc([PropertyPtr, DeclaringTypePtr](const TSharedRef<FFormulaValue>& Target, TSharedPtr<FFormulaValue>& Result) -> bool
	{
		FProperty* Property = PropertyPtr.Get();
		UField* DeclaringType = DeclaringTypePtr.Get();
		check(Property);
		check(DeclaringType);
		
		if (!Property || !DeclaringType)
		{
			return false;  // metadata is gone
		}

		if (const UClass* DeclaringClass = Cast<UClass>(DeclaringTypePtr.Get()))
		{
			const UObject* ContainerPtr = nullptr;
			if (Target->GetTypeCode() == EFormulaValueType::ObjectPtr &&
				Target->TryCopyCompleteValue(Target->GetType(), &ContainerPtr) &&
				!ContainerPtr->GetClass()->IsChildOf(DeclaringClass))
			{
				ContainerPtr = nullptr; // invalid target class
			}
			
			if (!ContainerPtr)
			{
				return false; // null reference target
			}
			
			const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(ContainerPtr);
			Result = MakeShared<FFormulaValue>(Property, ValuePtr);
			return true;
		}
		else if (const UScriptStruct* DeclaringStruct = Cast<UScriptStruct>(DeclaringTypePtr.Get()))
		{
			void* ContainerPtr = nullptr;
			if (Target->GetTypeCode() == EFormulaValueType::Struct &&
				Target->TryGetContainerAddress(ContainerPtr) &&
				GetScriptStruct(Target->GetType())->IsChildOf(DeclaringStruct))
			{
				ContainerPtr = nullptr; // invalid target class
			}
			
			if (!ContainerPtr)
			{
				return false; // null reference target
			}

			const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(ContainerPtr);
			Result = MakeShared<FFormulaValue>(Property, ValuePtr);
			return true;
		}
		return false;
	});
}

FFormulaPropertySetterFunc FFormulaProperty::CreateDefaultPropertySetter(TWeakFieldPtr<FProperty> PropertyPtr, TWeakObjectPtr<UField> DeclaringTypePtr)
{
	return FFormulaPropertySetterFunc([PropertyPtr, DeclaringTypePtr](const TSharedRef<FFormulaValue>& Target, const TSharedPtr<FFormulaValue>& Value) -> bool
	{
		FProperty* Property = PropertyPtr.Get();
		UField* DeclaringType = DeclaringTypePtr.Get();
		check(Property);
		check(DeclaringType);
		
		if (!Property || !DeclaringType)
		{
			return false; // metadata is gone
		}

		if (const UClass* DeclaringClass = Cast<UClass>(DeclaringType))
		{
			UObject* ContainerPtr = nullptr;
			if (Target->GetTypeCode() == EFormulaValueType::ObjectPtr &&
				Target->TryCopyCompleteValue(Target->GetType(), &ContainerPtr) &&
				!ContainerPtr->GetClass()->IsChildOf(DeclaringClass))
			{
				ContainerPtr = nullptr; // invalid target class
			}
		
			if (!ContainerPtr)
			{
				return false; // null reference target
			}
			return Value->TrySetPropertyValue_InContainer(Property, ContainerPtr, 0);
		}
		else if (const UScriptStruct* DeclaringStruct = Cast<UScriptStruct>(DeclaringType))
		{
			void* ContainerPtr = nullptr;
			if (Target->GetTypeCode() == EFormulaValueType::Struct &&
				Target->TryGetContainerAddress(ContainerPtr) &&
				GetScriptStruct(Target->GetType())->IsChildOf(DeclaringStruct))
			{
				ContainerPtr = nullptr; // invalid target class
			}
		
			if (!ContainerPtr)
			{
				return false; // null reference target
			}
		
			return Value->TrySetPropertyValue_InContainer(Property, ContainerPtr, 0);
		}
		return false;
	});
}
