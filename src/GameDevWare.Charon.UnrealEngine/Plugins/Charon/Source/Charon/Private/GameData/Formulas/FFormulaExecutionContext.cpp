// Copyright GameDevWare, Denis Zykov 2025

#include "GameData/Formulas/FFormulaExecutionContext.h"

FFormulaExecutionContext::FFormulaExecutionContext(
	const bool bAutoNullPropagation,
	const TMap<FString, const TSharedRef<FFormulaValue>>& Arguments,
    const TSharedRef<FFormulaValue>& Global,
    const TSharedRef<FFormulaTypeResolver>& TypeResolver
) :
	AutoNullPropagation(bAutoNullPropagation),
	Arguments(Arguments),
	Global(Global),
	TypeResolver(TypeResolver)
{
}
