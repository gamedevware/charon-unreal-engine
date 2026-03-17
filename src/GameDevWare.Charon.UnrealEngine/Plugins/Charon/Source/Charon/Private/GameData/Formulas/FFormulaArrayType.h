// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "FDotNetSurrogateType.h"
#include "GameData/Formulas/DotNetTypes/UDotNetArray.h"

class FFormulaArrayType : public FDotNetSurrogateType
{
private:
	TSharedRef<IFormulaType> const ElementType;
public:
	explicit FFormulaArrayType(const TSharedRef<IFormulaType>& ElementType)
		: FDotNetSurrogateType(UDotNetArray::StaticClass(), nullptr), ElementType(ElementType)
	{
	}

	virtual FString GetCPPType() const override { return TEXT("TArray<") + this->ElementType->GetCPPType() + TEXT(">"); }
	virtual TSharedPtr<IFormulaType> GetUnderlyingType() const override { return this->ElementType; }
};
