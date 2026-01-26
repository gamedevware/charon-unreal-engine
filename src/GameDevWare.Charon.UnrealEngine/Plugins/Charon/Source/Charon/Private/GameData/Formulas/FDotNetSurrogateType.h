// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "FFormulaUnrealType.h"

class FDotNetSurrogateType: public IFormulaType
{
private:
	TStrongObjectPtr<UClass> const ClassPtr;
	FProperty& LiteralField;
	EFormulaValueType const TypeCode;
		
	TSharedPtr<TArray<FString>> PropertyNames;
	TSharedPtr<TArray<FString>> FunctionNames;
	TSharedPtr<TArray<FString>> StaticPropertyNames;
	TSharedPtr<TArray<FString>> StaticFunctionNames;
	
	TSharedPtr<TMap<FString, FFormulaFunction>> Functions;
	TSharedPtr<TMap<FString, FFormulaProperty>> Properties;
	TSharedPtr<TMap<FString, FFormulaProperty>> StaticProperties;
	TSharedPtr<TMap<FString, FFormulaFunction>> StaticFunctions;
public:
	explicit FDotNetSurrogateType(UClass* SurrogateClass);
	virtual ~FDotNetSurrogateType() override { }

	virtual bool CanBeNull() const override;
	virtual bool IsAssignableFrom(UField* Type) const override;
	virtual UStruct* GetTypeClassOrStruct() const override { return ClassPtr.Get(); }
	virtual EFormulaValueType GetTypeCode() const override;
	virtual FString GetCPPType() const override;
	virtual TSharedPtr<IFormulaType> GetUnderlyingType() const override { return nullptr; }

	virtual const TArray<FString>& GetPropertyNames(bool bStatic) override;
	virtual const TArray<FString>& GetFunctionNames(bool bStatic) override;
	
	virtual bool TryGetBinaryOperation(EBinaryOperationType Operation, const FFormulaFunction*& FoundOperation) override { FoundOperation = nullptr; return false; }
	virtual bool TryGetUnaryOperation(EUnaryOperationType Operation, const FFormulaFunction*& FoundOperation) override { FoundOperation = nullptr; return false; }
	virtual bool TryGetConversionOperation(const FFormulaFunction*& FoundOperation) override { FoundOperation = nullptr; return false; }
	
	virtual bool TryGetFunction(const FString& MemberName, bool bStatic, const FFormulaFunction*& FoundFunction) override;
	virtual bool TryGetProperty(const FString& MemberName, bool bStatic, const FFormulaProperty*& FoundProperty) override;
};
