#pragma once
#include "GameData/Formulas/IFormulaTypeDescription.h"
#include "UObject/StrongObjectPtr.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "GameData/Formulas/Expressions/EBinaryOperationType.h"
#include "GameData/Formulas/Expressions/EUnaryOperationType.h"
#include "Misc/Optional.h"

class FFormulaFunction;

class FFormulaUnrealType : public IFormulaTypeDescription
{
private:
	TStrongObjectPtr<UStruct> StructOrClassPtr;
	
	TSharedPtr<TArray<FString>> PropertyNames;
	TSharedPtr<TArray<FString>> FunctionNames;
	TSharedPtr<TArray<FString>> StaticFunctionNames;
	
	TSharedPtr<TMap<FString, FFormulaFunction>> Functions;
	TSharedPtr<TMap<FString, FFormulaProperty>> Properties;
	TSharedPtr<TMap<FString, FFormulaProperty>> StaticProperties;
	TSharedPtr<TMap<FString, FFormulaFunction>> StaticFunctions;

	inline bool IsStruct() const { return !!Cast<UScriptStruct>(this->StructOrClassPtr.Get()); }
	inline bool IsClass() const { return !!Cast<UClass>(this->StructOrClassPtr.Get()); }
public:
	explicit FFormulaUnrealType(UStruct* StructPtr);
	virtual ~FFormulaUnrealType() override { }

	virtual bool CanBeNull() const override { return true; }
	virtual bool IsAssignableFrom(UField* Type) const override;
	virtual UField* GetTypeClassOrStruct() const override { return StructOrClassPtr.Get(); }
	virtual EFormulaValueType GetTypeCode() const override { return Cast<UClass>(StructOrClassPtr.Get()) ? EFormulaValueType::ObjectPtr : EFormulaValueType::Struct; }
	virtual FString GetCPPType() const override;
	virtual const TArray<FString>& GetPropertyNames(bool bStatic) override;
	virtual const TArray<FString>& GetFunctionNames(bool bStatic) override;

	virtual bool TryGetBinaryOperation(EBinaryOperationType Operation, const FFormulaFunction*& FoundOperations) override { return false; }
	virtual bool TryGetUnaryOperation(EUnaryOperationType Operation, const FFormulaFunction*& FoundMOperations) override { return false; }
	virtual bool TryGetFunction(const FString& MemberName, bool bStatic, const FFormulaFunction*& FoundFunction) override;
	virtual bool TryGetProperty(const FString& MemberName, bool bStatic, const FFormulaProperty*& FoundProperty) override;
	virtual bool TryGetConversionOperation(const FFormulaFunction*& FoundMember) override { return false; }
};
