// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "FFormulaExpression.h"
#include "../FFormulaTypeReference.h"

DECLARE_LOG_CATEGORY_EXTERN(LogNewExpression, Log, All);

class CHARON_API FNewExpression : public FFormulaExpression
{
public:
	const TMap<FString, TSharedPtr<FFormulaExpression>> Arguments;
	const TSharedPtr<FFormulaTypeReference> NewObjectType;
	
	explicit FNewExpression(const TSharedRef<FJsonObject>& ExpressionObj);
	explicit FNewExpression(const TSharedPtr<FFormulaTypeReference>& NewObjectType, const TMap<FString, TSharedPtr<FFormulaExpression>>& Arguments);

	virtual FFormulaExecutionResult Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const override;

	inline static EFormulaExpressionType Type = EFormulaExpressionType::NewExpression;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
	virtual bool IsValid() const override;
	virtual void DebugPrintTo(FString& OutValue) const override;
	
	static bool TryCreateArray(const TSharedPtr<IFormulaType>& ElementType, FArrayProperty* ArrayProperty, TSharedPtr<FFormulaValue>& OutCreatedArray);
private:
	FFormulaExecutionResult CreateNewArray(const TSharedPtr<FFormulaTypeReference>& ElementTypeRef,	const FFormulaExecutionContext& Context, FProperty* ExpectedType) const;
	FFormulaExecutionResult CreateNewSet(const TSharedPtr<FFormulaTypeReference>& ElementTypeRef, const FFormulaExecutionContext& Context, FProperty* ExpectedType) const;
	FFormulaExecutionResult CreateNewMap(const TSharedPtr<FFormulaTypeReference>& KeyTypeRef, const TSharedPtr<FFormulaTypeReference>& ValueTypeRef, const FFormulaExecutionContext& Context, FProperty* ExpectedType) const;
	FFormulaExecutionResult CreatePrimitiveObject(const TSharedPtr<IFormulaType>& NewObjectTypeRef) const;
	
};
