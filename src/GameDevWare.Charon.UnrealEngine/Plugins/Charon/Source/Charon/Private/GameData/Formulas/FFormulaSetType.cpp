// Copyright GameDevWare, Denis Zykov 2025

// ReSharper disable CppUseFamiliarTemplateSyntaxForGenericLambdas
#include "FFormulaSetType.h"
#include "GameData/Formulas/FFormulaProperty.h"
#include "GameData/Formulas/DotNetTypes/UDotNetInt32.h"
#include "UObject/UnrealType.h"

void FFormulaSetType::InitializePropertyNames(UClass* DeclaringClass, const TSharedRef<TArray<FString>> NameList)
{
	FDotNetSurrogateType::InitializePropertyNames(DeclaringClass, NameList);
	NameList->Add(TEXT("Count"));
}

void FFormulaSetType::InitializeProperties(UClass* DeclaringClass, const TSharedRef<TMap<FString, FFormulaProperty>> PropertyList)
{
	FDotNetSurrogateType::InitializeProperties(DeclaringClass, PropertyList);

	FFormulaPropertyGetterFunc CountGetter = [](const TSharedRef<FFormulaValue>& Target, TSharedPtr<FFormulaValue>& OutValue) -> bool
	{
		const FSetProperty* SetProp = CastField<FSetProperty>(Target->GetType());
		if (!SetProp)
		{
			return false;
		}
		void* ContainerPtr = nullptr;
		if (!Target->TryGetContainerAddress(ContainerPtr) || !ContainerPtr)
		{
			return false;
		}
		FScriptSetHelper SetHelper(SetProp, ContainerPtr);
		OutValue = MakeShared<FFormulaValue>(SetHelper.Num());
		return true;
	};

	PropertyList->Add(TEXT("Count"), FFormulaProperty(UDotNetInt32::GetLiteralProperty(), CountGetter, FFormulaPropertySetterFunc{}, DeclaringClass, /*bUseClassDefaultObject*/ false));
}