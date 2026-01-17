#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "Templates/SharedPointer.h"
#include "Containers/Map.h"
#include "Containers/Array.h"
#include "EFormulaExpressionType.h"
#include "FFormulaExecutionContext.h"
#include "FFormulaInvokeResult.h"
#include "FFormulaValue.h"
#include "Dom/JsonObject.h"

class CHARON_API FFormulaExpression : public TSharedFromThis<FFormulaExpression>
{
public:
	virtual ~FFormulaExpression() = default;

	virtual EFormulaExpressionType GetType() const = 0;
	virtual bool IsNullPropagationEnabled() const { return false; }

	virtual FFormulaInvokeResult Invoke(const FFormulaExecutionContext& Context)
	{
		return MakeValueResult(FFormulaValue::Null());
	}
protected:
	static FFormulaInvokeResult MakeValueResult(const FFormulaValue& Value)
	{
		FFormulaInvokeResult Result;
		Result.Set<FFormulaValue>(Value);
		return Result;
	}
	static FFormulaInvokeResult MakeErrorResult(const FFormulaInvokeError& Error)
	{
		FFormulaInvokeResult Result;
		Result.Set<FFormulaInvokeError>(Error);
		return Result;
	}
};