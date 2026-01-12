#include "GameData/Formulas/FFormulaExecutionContext.h"

FFormulaExecutionContext::FFormulaExecutionContext(
	const bool bAutoNullPropagation,
	const TMap<FString, FFormulaVariableValue>& Arguments,
    const FFormulaVariableValue& Global,
    const TSharedRef<FFormulaTypeResolver>& TypeResolver
) : AutoNullPropagation(bAutoNullPropagation), Arguments(Arguments), Global(Global), TypeResolver(TypeResolver)
{
}
