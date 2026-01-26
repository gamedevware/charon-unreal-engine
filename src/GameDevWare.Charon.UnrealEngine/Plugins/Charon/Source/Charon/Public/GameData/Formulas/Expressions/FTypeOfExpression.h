// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "FFormulaExpression.h"
#include "../FFormulaTypeReference.h"

class CHARON_API FTypeOfExpression final : public FFormulaExpression
{
public:
	const TSharedPtr<FFormulaTypeReference> TypeRef;
	
	explicit FTypeOfExpression(const TSharedRef<FJsonObject>& ExpressionObj);
	explicit FTypeOfExpression(const TSharedPtr<FFormulaTypeReference>& TypeRef);

	virtual FFormulaExecutionResult Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const override;

	inline static EFormulaExpressionType Type = EFormulaExpressionType::TypeOfExpression;
	virtual EFormulaExpressionType GetType() const override  { return Type; }
	virtual bool IsValid() const override;
	virtual void DebugPrintTo(FString& OutValue) const override;
};
