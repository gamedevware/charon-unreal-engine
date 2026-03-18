// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "FFormulaUnrealType.h"

#define GET_LITERAL_MEMBER_CHECKED(ClassName) \
*ClassName::StaticClass()->FindPropertyByName(GET_MEMBER_NAME_STRING_CHECKED(ClassName, __Literal))

class FDotNetSurrogateType: public IFormulaType
{
private:
	TStrongObjectPtr<UClass> const ClassPtr;
	FProperty* LiteralField;
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
	explicit FDotNetSurrogateType(UClass* SurrogateClass, FProperty* LiteralProperty);
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

	static FFormulaFunction GetRefenceEqualsFunction(UField* DeclaringType);
	static FFormulaFunction GetValueEqualsFunction(UField* DeclaringType);
	static FFormulaFunction GetToStringFunction(UField* DeclaringType);
	static FFormulaFunction GetGetTypeFunction(UField* DeclaringType);
	static FFormulaFunction GetEqualsFunction(UField* DeclaringType);

protected:
	virtual void InitializeStaticFunctions(UClass* DeclaringClass, const TSharedRef<TMap<FString, FFormulaFunction>> FunctionList);
	virtual void InitializeFunctions(UClass* DeclaringClass, const TSharedRef<TMap<FString, FFormulaFunction>> FunctionList);
	virtual void InitializeStaticFunctionNames(UClass* DeclaringClass, const TSharedRef<TArray<FString>> NameList);
	virtual void InitializeFunctionNames(UClass* DeclaringClass, const TSharedRef<TArray<FString>> NameList);
	
	virtual void InitializeStaticProperties(UClass* DeclaringClass, const TSharedRef<TMap<FString, FFormulaProperty>> PropertyList);
	virtual void InitializeProperties(UClass* DeclaringClass, const TSharedRef<TMap<FString, FFormulaProperty>> PropertyList);
	virtual void InitializeStaticPropertyNames(UClass* DeclaringClass, const TSharedRef<TArray<FString>> NameList);
	virtual void InitializePropertyNames(UClass* DeclaringClass, const TSharedRef<TArray<FString>> NameList);
};
