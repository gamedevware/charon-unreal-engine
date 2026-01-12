#pragma once

#include "CoreMinimal.h"
#include "EFormulaExpressionType.h"
#include "FFormulaExecutionContext.h"
#include "FFormulaVariableValue.h"
#include "Dom/JsonObject.h"

class CHARON_API FFormulaExpression : public TSharedFromThis<FFormulaExpression>
{
public:
	virtual ~FFormulaExpression() = default;

	virtual EFormulaExpressionType GetType() const = 0;
	virtual bool IsNullPropagationEnabled() const { return false; }

	virtual FFormulaVariableValue Invoke(FFormulaExecutionContext Context) { return FFormulaVariableValue(); }
};