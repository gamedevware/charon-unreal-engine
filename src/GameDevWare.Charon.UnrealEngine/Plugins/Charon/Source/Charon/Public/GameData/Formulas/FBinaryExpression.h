#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "FFormulaExpression.h"
#include "EBinaryOperationType.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBinaryExpression, Log, All);

class CHARON_API FBinaryExpression : public FFormulaExpression
{
private:
    TSharedPtr<FFormulaExpression> Left;
    TSharedPtr<FFormulaExpression> Right;
    EBinaryOperationType BinaryOperationType;
    EBinaryOperationType FallbackBinaryOperationType;
    
    static EBinaryOperationType MapToBinaryOperationType(const FString& ExpressionType);

public:
    explicit FBinaryExpression(const TSharedRef<FJsonObject>& ExpressionObj);

    virtual EFormulaExpressionType GetType() const override  { return EFormulaExpressionType::BinaryExpression; }
};