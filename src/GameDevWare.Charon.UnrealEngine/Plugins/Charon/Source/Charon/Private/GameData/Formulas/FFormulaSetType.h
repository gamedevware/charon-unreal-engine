// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "FDotNetSurrogateType.h"
#include "GameData/Formulas/DotNetTypes/UDotNetHashSet.h"

class FFormulaSetType : public FDotNetSurrogateType
{
private:
	TSharedRef<IFormulaType> const ElementType;
public:
	explicit FFormulaSetType(const TSharedRef<IFormulaType>& ElementType)
		: FDotNetSurrogateType(UDotNetHashSet::StaticClass(), nullptr), ElementType(ElementType)
	{
	}

	virtual FString GetCPPType() const override { return TEXT("TSet<") + this->ElementType->GetCPPType() + TEXT(">"); }
	virtual TSharedPtr<IFormulaType> GetUnderlyingType() const override { return this->ElementType; }

protected:
	virtual void InitializeProperties(UClass* DeclaringClass, const TSharedRef<TMap<FString, FFormulaProperty>> PropertyList) override;
	virtual void InitializePropertyNames(UClass* DeclaringClass, const TSharedRef<TArray<FString>> NameList) override;
};
