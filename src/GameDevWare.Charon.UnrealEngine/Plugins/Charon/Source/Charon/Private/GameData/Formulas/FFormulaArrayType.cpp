// Copyright GameDevWare, Denis Zykov 2025

// ReSharper disable CppUseFamiliarTemplateSyntaxForGenericLambdas
#include "FFormulaArrayType.h"
#include "GameData/Formulas/FFormulaProperty.h"
#include "GameData/Formulas/DotNetTypes/UDotNetInt32.h"
#include "UObject/UnrealType.h"

void FFormulaArrayType::InitializePropertyNames(UClass* DeclaringClass, const TSharedRef<TArray<FString>> NameList)
{
	FDotNetSurrogateType::InitializePropertyNames(DeclaringClass, NameList);
	NameList->Add(TEXT("Count"));
	NameList->Add(TEXT("Length"));
}

void FFormulaArrayType::InitializeProperties(UClass* DeclaringClass, const TSharedRef<TMap<FString, FFormulaProperty>> PropertyList)
{
	FDotNetSurrogateType::InitializeProperties(DeclaringClass, PropertyList);

	FFormulaPropertyGetterFunc CountGetter = [](const TSharedRef<FFormulaValue>& Target, TSharedPtr<FFormulaValue>& OutValue) -> bool
	{
		const FArrayProperty* ArrayProp = CastField<FArrayProperty>(Target->GetType());
		if (!ArrayProp)
		{
			return false;
		}
		void* ContainerPtr = nullptr;
		if (!Target->TryGetContainerAddress(ContainerPtr) || !ContainerPtr)
		{
			return false;
		}
		FScriptArrayHelper ArrayHelper(ArrayProp, ContainerPtr);
		OutValue = MakeShared<FFormulaValue>(ArrayHelper.Num());
		return true;
	};

	const FFormulaProperty CountProperty(UDotNetInt32::GetLiteralProperty(), CountGetter, FFormulaPropertySetterFunc{}, DeclaringClass, /*bUseClassDefaultObject*/ false);
	PropertyList->Add(TEXT("Count"), CountProperty);
	PropertyList->Add(TEXT("Length"), CountProperty);
}