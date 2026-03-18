// Copyright GameDevWare, Denis Zykov 2025

// ReSharper disable CppUseFamiliarTemplateSyntaxForGenericLambdas
#include "FFormulaMapType.h"
#include "GameData/Formulas/FFormulaProperty.h"
#include "GameData/Formulas/DotNetTypes/UDotNetInt32.h"
#include "UObject/UnrealType.h"

void FFormulaMapType::InitializePropertyNames(UClass* DeclaringClass, const TSharedRef<TArray<FString>> NameList)
{
	FDotNetSurrogateType::InitializePropertyNames(DeclaringClass, NameList);
	NameList->Add(TEXT("Count"));
}

void FFormulaMapType::InitializeProperties(UClass* DeclaringClass, const TSharedRef<TMap<FString, FFormulaProperty>> PropertyList)
{
	FDotNetSurrogateType::InitializeProperties(DeclaringClass, PropertyList);

	FFormulaPropertyGetterFunc CountGetter = [](const TSharedRef<FFormulaValue>& Target, TSharedPtr<FFormulaValue>& OutValue) -> bool
	{
		const FMapProperty* MapProp = CastField<FMapProperty>(Target->GetType());
		if (!MapProp)
		{
			return false;
		}
		void* ContainerPtr = nullptr;
		if (!Target->TryGetContainerAddress(ContainerPtr) || !ContainerPtr)
		{
			return false;
		}
		FScriptMapHelper MapHelper(MapProp, ContainerPtr);
		OutValue = MakeShared<FFormulaValue>(MapHelper.Num());
		return true;
	};

	PropertyList->Add(TEXT("Count"), FFormulaProperty(UDotNetInt32::GetLiteralProperty(), CountGetter, FFormulaPropertySetterFunc{}, DeclaringClass, /*bUseClassDefaultObject*/ false));
}