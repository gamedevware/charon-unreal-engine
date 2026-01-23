#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "../EFormulaExpressionType.h"
#include "../FFormulaExecutionContext.h"
#include "../FFormulaInvokeResult.h"
#include "../FFormulaValue.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Dom/JsonObject.h"
#include "Templates/SharedPointer.h"

class CHARON_API FFormulaExpression : public TSharedFromThis<FFormulaExpression>
{
public:
	virtual ~FFormulaExpression() = default;

	virtual EFormulaExpressionType GetType() const = 0;
	virtual bool IsNullPropagationEnabled() const { return false; }

	virtual FFormulaInvokeResult Execute(const FFormulaExecutionContext& Context ) const
	{
		return FFormulaValue::Null();
	}
protected:
};

template <typename ExpressionT>
ExpressionT* CastExpression(FFormulaExpression* Expression)
{
	// 1. Ensure Expression isn't null
	// 2. Compare instance type to the template's static Type
	if (Expression && Expression->GetType() == ExpressionT::Type)
	{
		return static_cast<ExpressionT*>(Expression);
	}
	return nullptr;
}