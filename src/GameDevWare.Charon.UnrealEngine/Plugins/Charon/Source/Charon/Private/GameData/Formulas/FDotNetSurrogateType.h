#pragma once
#include "FFormulaUnrealType.h"

class FDotNetSurrogateType: public IFormulaTypeDescription
{
private:
	const TStrongObjectPtr<UClass> ClassPtr;
	const FProperty* LiteralField;
	EFormulaValueType TypeCode;
		
	TSharedPtr<TArray<FString>> PropertyNames;
	TSharedPtr<TArray<FString>> FunctionNames;
	TSharedPtr<TArray<FString>> StaticPropertyNames;
	TSharedPtr<TArray<FString>> StaticFunctionNames;
	
	TSharedPtr<TMap<FString, FFormulaFunction>> Functions;
	TSharedPtr<TMap<FString, FFormulaProperty>> Properties;
	TSharedPtr<TMap<FString, FFormulaProperty>> StaticProperties;
	TSharedPtr<TMap<FString, FFormulaFunction>> StaticFunctions;
public:
	FDotNetSurrogateType(UClass* SurrogateClass);
	virtual ~FDotNetSurrogateType() override { }

	virtual bool CanBeNull() const override;
	virtual bool IsAssignableFrom(UField* Type) const override;
	virtual UStruct* GetTypeClassOrStruct() const override { return ClassPtr.Get(); }
	virtual EFormulaValueType GetTypeCode() const override;
	virtual FString GetCPPType() const override;
	virtual const TArray<FString>& GetPropertyNames(bool bStatic) override;
	virtual const TArray<FString>& GetFunctionNames(bool bStatic) override;

	virtual bool TryGetBinaryOperation(EBinaryOperationType Operation, const FFormulaFunction*& FoundOperation);
	virtual bool TryGetUnaryOperation(EUnaryOperationType Operation, const FFormulaFunction*& FoundOperation);
	virtual bool TryGetConversionOperation(const FFormulaFunction*& FoundOperation) override;
	
	virtual bool TryGetFunction(const FString& MemberName, bool bStatic, const FFormulaFunction*& FoundFunction);
	virtual bool TryGetProperty(const FString& MemberName, bool bStatic, const FFormulaProperty*& FoundProperty);
};
