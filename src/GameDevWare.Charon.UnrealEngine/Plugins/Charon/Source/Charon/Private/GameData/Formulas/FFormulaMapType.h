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
		FDotNetSurrogateType(UDotNetDictionary::StaticClass(), nullptr), 
		KeyType(KeyType), 
		ValueType(ValueType) { }

	virtual FString GetCPPType() const override { return TEXT("TMap<") + this->KeyType->GetCPPType() + TEXT(",") + this->ValueType->GetCPPType() + TEXT(">"); }
	virtual TSharedPtr<IFormulaType> GetUnderlyingType() const override { return this->ValueType; }
};
