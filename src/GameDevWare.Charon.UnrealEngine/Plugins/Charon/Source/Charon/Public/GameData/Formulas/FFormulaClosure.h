// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "FFormulaExecutionContext.h"
#include "FFormulaExecutionResult.h"
#include "FFormulaTypeResolver.h"
#include "FFormulaValue.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Templates/SharedPointer.h"
#include "Expressions/FFormulaExpression.h"

/**
 * A lambda expression captured as an invocable value.
 *
 * Produced by FLambdaExpression::Execute and carried through the interpreter inside an
 * FDotNetFuncValue struct value, which lets a lambda be passed as an ordinary call argument
 * without changing the eager argument evaluation in FInvokeExpression.
 *
 * Like FFormulaValue, its lifetime is a single expression evaluation round.
 */
class CHARON_API FFormulaClosure : public TSharedFromThis<FFormulaClosure>
{
public:
	/** Lambda body, evaluated on each Invoke. */
	TSharedPtr<FFormulaExpression> const Body;
	/** Lambda parameter names, bound positionally from Invoke arguments. */
	TArray<FString> const ParameterNames;
	/** Null propagation mode of the declaring context. */
	bool const bAutoNullPropagation;
	/** Arguments of the scope the lambda was declared in. Parameters shadow these. */
	TMap<FString, const TSharedRef<FFormulaValue>> const CapturedArguments;
	/** Global ("this") scope of the declaring context. */
	TSharedRef<FFormulaValue> const CapturedGlobal;
	/** Type resolver of the declaring context. */
	TSharedRef<FFormulaTypeResolver> const CapturedTypeResolver;

	FFormulaClosure(
		const TSharedPtr<FFormulaExpression>& Body,
		const TArray<FString>& ParameterNames,
		const FFormulaExecutionContext& DeclaringContext
	);

	/**
	 * Evaluates the lambda body with Arguments bound positionally to ParameterNames,
	 * over the scope captured when the lambda was declared.
	 */
	FFormulaExecutionResult Invoke(TArrayView<const TSharedRef<FFormulaValue>> Arguments) const;

	/** Wraps a closure into an FFormulaValue carrying an FDotNetFuncValue. */
	static TSharedRef<FFormulaValue> ToValue(const TSharedRef<FFormulaClosure>& Closure);

	/** Extracts a closure from a value produced by ToValue. Returns false for any other value. */
	static bool TryGetFrom(const TSharedRef<FFormulaValue>& Value, TSharedPtr<FFormulaClosure>& OutClosure);
};
