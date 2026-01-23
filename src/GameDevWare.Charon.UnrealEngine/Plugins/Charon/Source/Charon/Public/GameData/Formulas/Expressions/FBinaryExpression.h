#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "FFormulaExpression.h"
#include "EBinaryOperationType.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBinaryExpression, Log, All);

class CHARON_API FBinaryExpression : public FFormulaExpression
{
public:
   const  TSharedPtr<FFormulaExpression> Left;
    const TSharedPtr<FFormulaExpression> Right;
   const  EBinaryOperationType BinaryOperationType;
    
    explicit FBinaryExpression(const TSharedRef<FJsonObject>& ExpressionObj);

	virtual FFormulaInvokeResult Execute(const FFormulaExecutionContext& Context) const override;
	
	inline static EFormulaExpressionType Type = EFormulaExpressionType::BinaryExpression;
    virtual EFormulaExpressionType GetType() const override  { return Type; }

private:
	FFormulaInvokeResult ExecuteCoalesce(const FFormulaExecutionContext& Context) const;
	FFormulaInvokeResult ExecuteJunction(const FFormulaExecutionContext& Context) const;
	FFormulaInvokeResult ExecutePower(const TSharedPtr<FFormulaValue>& LeftOperand,	const TSharedPtr<FFormulaValue>& RightOperand) const;
	FFormulaInvokeResult ExecuteNullLiftedBoolean(const TSharedPtr<FFormulaValue>& LeftOperand, const TSharedPtr<FFormulaValue>& RightOperand) const;
	FFormulaInvokeResult ExecuteNullLifted(const TSharedPtr<FFormulaValue>& LeftOperand, const TSharedPtr<FFormulaValue>& RightOperand) const;
};