#pragma once

#include "FFormulaExpression.h"

class CHARON_API FMemberExpression : public FFormulaExpression
{
private:
	bool bUseNullPropagation;
	TSharedPtr<FFormulaExpression> Expression;
	FString RawMemberName;
	FString MemberName;
	TArray<TSharedPtr<class FFormulaTypeReference>> TypeArguments;

public:
	explicit FMemberExpression(const TSharedRef<FJsonObject>& ExpressionObj);

	virtual EFormulaExpressionType GetType() const override  { return EFormulaExpressionType::MemberExpression; }
	
	virtual bool IsNullPropagationEnabled() const override;

	FString GetMemberName() const { return MemberName; }
};
