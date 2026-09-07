// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "FDotNetEnumerableType.h"
#include "GameData/Formulas/DotNetTypes/UDotNetArray.h"

class FFormulaArrayType : public FDotNetEnumerableType
{
public:
	explicit FFormulaArrayType(const TSharedRef<IFormulaType>& ElementType)
		: FDotNetEnumerableType(UDotNetArray::StaticClass(), nullptr, ElementType)
	{
	}

	virtual FString GetCPPType() const override { return TEXT("TArray<") + this->ElementType->GetCPPType() + TEXT(">"); }

protected:
	virtual void InitializeProperties(UClass* DeclaringClass, const TSharedRef<TMap<FString, FFormulaProperty>> PropertyList) override;
	virtual void InitializePropertyNames(UClass* DeclaringClass, const TSharedRef<TArray<FString>> NameList) override;
};
