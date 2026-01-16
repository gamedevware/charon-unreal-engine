#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "EBinaryOperationType.h"
#include "EUnaryOperationType.h"
#include "FFormulaMethodGroup.h"
#include "FFormulaVariableValue.h"
#include "Containers/Array.h"
#include "Templates/SharedPointer.h"

class CHARON_API IFormulaTypeDescription : public TSharedFromThis<IFormulaTypeDescription>
{
public:
	virtual ~IFormulaTypeDescription() = default;
	
	virtual bool CanBeNull() = 0; 
	virtual FFormulaVariableValue GetDefaultValue() = 0;
	virtual const TArray<FString>& GetPropertyNames(bool bStatic) = 0;
	virtual const TArray<FString>& GetFunctionNames(bool bStatic) = 0;

	virtual bool TryGetBinaryOperation(EBinaryOperationType Operation, FFormulaMethodGroup*& FoundMOperations) = 0;
	virtual bool TryGetUnaryOperation(EUnaryOperationType Operation, FFormulaMethodGroup*& FoundMOperations) = 0;
	virtual bool TryGetMethod(const FString& MemberName, bool bStatic, FFormulaMethodGroup*& FoundMember) = 0;
	virtual bool TryGetProperty(const FString& MemberName, bool bStatic, FProperty*& FoundMember) = 0;
	virtual bool TryGetIndexer(int Rank, FFormulaMethodGroup*& FoundMember) = 0;
	virtual bool TryGetConversionOperation(FFormulaMethodGroup*& FoundMember) = 0;
};
