#pragma once
#include "GameData/Formulas/IFormulaTypeDescription.h"
#include "UObject/StrongObjectPtr.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Misc/Optional.h"

class FFormulaFunction;

class FFormulaEnumDescription : public IFormulaTypeDescription
{
private:
	TStrongObjectPtr<UEnum> EnumPtr;
	TArray<FString> EnumLiteralNames;
	TArray<FString> FunctionNames;
	TMap<FString, FFormulaFunction> Functions;
	TMap<FString, FFormulaProperty> EnumLiterals;
public:
	explicit FFormulaEnumDescription(UEnum* Enum);
	virtual ~FFormulaEnumDescription() override { }

	virtual bool CanBeNull() const override { return false; }
	virtual bool IsAssignableFrom(UField* Type) const override;
	virtual UField* GetTypeClassOrStruct() const override { return this->EnumPtr.Get(); }
	virtual EFormulaValueType GetTypeCode() const override { return EFormulaValueType::Enum; }
	virtual FString GetCPPType() const override;
	virtual const TArray<FString>& GetPropertyNames(bool bStatic) override;
	virtual const TArray<FString>& GetFunctionNames(bool bStatic) override;

	virtual bool TryGetBinaryOperation(EBinaryOperationType Operation, const FFormulaFunction*& FoundOperations) override { return false; }
	virtual bool TryGetUnaryOperation(EUnaryOperationType Operation, const FFormulaFunction*& FoundMOperations) override { return false; }
	virtual bool TryGetFunction(const FString& MemberName, bool bStatic, const FFormulaFunction*& FoundFunction) override;
	virtual bool TryGetProperty(const FString& MemberName, bool bStatic, const FFormulaProperty*& FoundMember) override;
	virtual bool TryGetConversionOperation(const FFormulaFunction*& FoundMember) override { return false; }

private:
	bool EnumToString(const TSharedRef<FFormulaValue>& Target, TSharedPtr<FFormulaValue>& Result) const;
};
