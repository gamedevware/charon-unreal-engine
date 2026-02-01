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
		: FDotNetSurrogateType(UDotNetHashSet::StaticClass()), ElementType(ElementType)
	{
	}

	virtual TSharedPtr<IFormulaType> GetUnderlyingType() const override { return this->ElementType; }
};
