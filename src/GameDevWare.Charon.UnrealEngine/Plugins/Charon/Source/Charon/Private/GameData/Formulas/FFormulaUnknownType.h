// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "GameData/Formulas/IFormulaType.h"
#include "UObject/StrongObjectPtr.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "GameData/Formulas/Expressions/EBinaryOperationType.h"
#include "GameData/Formulas/Expressions/EUnaryOperationType.h"
#include "Misc/Optional.h"

class FFormulaFunction;

static TArray<FString> EmptyNames;
class FFormulaUnknownType : public IFormulaType
{
private:
	TWeakFieldPtr<const FProperty> const PropertyPtr;

public:
	explicit FFormulaUnknownType(const FProperty* Property) :
		PropertyPtr(TWeakFieldPtr<const FProperty>(Property)) { }
	virtual ~FFormulaUnknownType() override { }

	virtual bool CanBeNull() const override { return true; }
	virtual bool IsAssignableFrom(UField* Type) const override { return false; }
	virtual UField* GetTypeClassOrStruct() const override { return nullptr; }
	virtual EFormulaValueType GetTypeCode() const override { return EFormulaValueType::Struct; }
	virtual FString GetCPPType() const override { return PropertyPtr.IsValid() ? PropertyPtr->GetCPPType() : TEXT("Unknown"); }
	virtual TSharedPtr<IFormulaType> GetUnderlyingType() const override { return nullptr; }
	
	virtual const TArray<FString>& GetPropertyNames(bool bStatic) override { return EmptyNames; }
	virtual const TArray<FString>& GetFunctionNames(bool bStatic) override { return EmptyNames; }

	virtual bool TryGetBinaryOperation(EBinaryOperationType Operation, const FFormulaFunction*& FoundOperation) override { FoundOperation = nullptr; return false; }
	virtual bool TryGetUnaryOperation(EUnaryOperationType Operation, const FFormulaFunction*& FoundOperation) override { FoundOperation = nullptr; return false; }
	virtual bool TryGetFunction(const FString& MemberName, bool bStatic, const FFormulaFunction*& FoundFunction) override { FoundFunction = nullptr; return false; }
	virtual bool TryGetProperty(const FString& MemberName, bool bStatic, const FFormulaProperty*& FoundProperty) override { FoundProperty = nullptr; return false; }
	virtual bool TryGetConversionOperation(const FFormulaFunction*& FoundMOperation) override { FoundMOperation = nullptr; return false; }
};
