#include "GameData/Formulas/FFormulaExecutionContext.h"

FFormulaExecutionContext::FFormulaExecutionContext(
	const bool bAutoNullPropagation,
	const TMap<FString, FFormulaValue>& Arguments,
    const FFormulaValue& Global,
    const TSharedRef<FFormulaTypeResolver>& TypeResolver
) : AutoNullPropagation(bAutoNullPropagation), Arguments(Arguments), Global(Global), TypeResolver(TypeResolver)
{
}
