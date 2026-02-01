// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "FDotNetSurrogateType.h"
#include "GameData/Formulas/DotNetTypes/UDotNetDictionary.h"

class FFormulaMapType : public FDotNetSurrogateType
{
private:
	TSharedRef<IFormulaType> const KeyType;
	TSharedRef<IFormulaType> const ValueType;
public:
	explicit FFormulaMapType(const TSharedRef<IFormulaType>& KeyType, const TSharedRef<IFormulaType>& ValueType) : 
		FDotNetSurrogateType(UDotNetDictionary::StaticClass()), 
		KeyType(KeyType), 
		ValueType(ValueType) { }

	virtual TSharedPtr<IFormulaType> GetUnderlyingType() const override { return this->ValueType; }
};
