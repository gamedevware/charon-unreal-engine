// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "FFormulaFunction.h"
#include "FFormulaProperty.h"
#include "FFormulaValue.h"
#include "Containers/Array.h"
#include "Expressions/EBinaryOperationType.h"
#include "Expressions/EUnaryOperationType.h"
#include "Templates/SharedPointer.h"
#include "UObject/Class.h"

class CHARON_API IFormulaType : public TSharedFromThis<IFormulaType>
{
public:
	virtual ~IFormulaType() = default;
	
	virtual bool CanBeNull() const = 0;
	virtual bool IsAssignableFrom(UField* Type) const = 0;
	virtual bool IsAssignableFrom(const FProperty* ToType, bool bWithoutConversion);
	virtual FString GetCPPType() const = 0;
	virtual UField* GetTypeClassOrStruct() const = 0;
	virtual EFormulaValueType GetTypeCode() const = 0;
	virtual TSharedPtr<IFormulaType> GetUnderlyingType() const = 0;

	virtual const TArray<FString>& GetPropertyNames(bool bStatic) = 0;
	virtual const TArray<FString>& GetFunctionNames(bool bStatic) = 0;

	virtual bool TryGetBinaryOperation(EBinaryOperationType Operation, const FFormulaFunction*& FoundMOperation) = 0;
	virtual bool TryGetUnaryOperation(EUnaryOperationType Operation, const FFormulaFunction*& FoundMOperation) = 0;
	virtual bool TryGetConversionOperation(const FFormulaFunction*& FoundMOperation) = 0;

	virtual bool TryGetFunction(const FString& MemberName, bool bStatic, const FFormulaFunction*& FoundFunction) = 0;
	virtual bool TryGetProperty(const FString& MemberName, bool bStatic, const FFormulaProperty*& FoundProperty) = 0;
};
