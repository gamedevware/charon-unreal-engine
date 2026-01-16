#pragma once
#include "GameData/Formulas/IFormulaTypeDescription.h"
#include "UObject/StrongObjectPtr.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Misc/Optional.h"

class FFormulaMethodGroup;

class FFormulaUClassDescription : public IFormulaTypeDescription
{
private:
	TStrongObjectPtr<UClass> ClassPtr;
	TOptional<TArray<FString>> ClassPropertyNames;
	TOptional<TArray<FString>> ClassFunctionNames;
	TMap<FString, FFormulaMethodGroup> BoundFunctions;
	TMap<FString, TFieldPath<FProperty>> BoundProperties;
public:
	explicit FFormulaUClassDescription(UClass* ClassPtr);
	virtual ~FFormulaUClassDescription() override { }

	virtual bool CanBeNull() override { return true; }
	virtual FFormulaVariableValue GetDefaultValue() override;
	virtual const TArray<FString>& GetPropertyNames(bool bStatic) override;
	virtual const TArray<FString>& GetFunctionNames(bool bStatic) override;

	virtual bool TryGetBinaryOperation(EBinaryOperationType Operation, FFormulaMethodGroup*& FoundOperations) override;
	virtual bool TryGetUnaryOperation(EUnaryOperationType Operation, FFormulaMethodGroup*& FoundMOperations) override;
	virtual bool TryGetMethod(const FString& MemberName, bool bStatic, FFormulaMethodGroup*& FoundMember) override;
	virtual bool TryGetProperty(const FString& MemberName, bool bStatic, FProperty*& FoundMember) override;
	virtual bool TryGetIndexer(int Rank, FFormulaMethodGroup*& FoundMember) override;
	virtual bool TryGetConversionOperation(FFormulaMethodGroup*& FoundMember) override;
};
