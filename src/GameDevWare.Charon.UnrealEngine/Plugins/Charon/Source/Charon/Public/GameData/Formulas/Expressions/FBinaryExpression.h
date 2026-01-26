// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "FFormulaExpression.h"
#include "EBinaryOperationType.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBinaryExpression, Log, All);

class CHARON_API FBinaryExpression : public FFormulaExpression
{
public:
   TSharedPtr<FFormulaExpression> const Left;
   TSharedPtr<FFormulaExpression> const Right;
   EBinaryOperationType const BinaryOperationType;
    
    explicit FBinaryExpression(const TSharedRef<FJsonObject>& ExpressionObj);
	FBinaryExpression(
		const TSharedPtr<FFormulaExpression>& Left,
		const TSharedPtr<FFormulaExpression>& Right,
		const EBinaryOperationType BinaryOperationType
	);

	virtual FFormulaExecutionResult Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const override;
	
	inline static EFormulaExpressionType Type = EFormulaExpressionType::BinaryExpression;
    virtual EFormulaExpressionType GetType() const override  { return Type; }
	virtual bool IsValid() const override;
	virtual void DebugPrintTo(FString& OutValue) const override;
	
private:
	FFormulaExecutionResult ExecuteCoalesce(const FFormulaExecutionContext& Context) const;
	FFormulaExecutionResult ExecuteJunction(const FFormulaExecutionContext& Context) const;
	FFormulaExecutionResult ExecutePower(const TSharedPtr<FFormulaValue>& LeftOperand,	const TSharedPtr<FFormulaValue>& RightOperand) const;
	FFormulaExecutionResult ExecuteNullLiftedBoolean(const TSharedPtr<FFormulaValue>& LeftOperand, const TSharedPtr<FFormulaValue>& RightOperand) const;
	FFormulaExecutionResult ExecuteNullLifted(const TSharedPtr<FFormulaValue>& LeftOperand, const TSharedPtr<FFormulaValue>& RightOperand) const;
};