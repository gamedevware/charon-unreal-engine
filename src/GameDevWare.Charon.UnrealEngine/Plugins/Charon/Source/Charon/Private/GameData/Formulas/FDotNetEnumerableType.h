// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "FDotNetSurrogateType.h"

/**
 * Holds the System.Linq.Enumerable method surface once, for every enumerable formula type.
 *
 * FFormulaArrayType derives from this, so the LINQ methods are declared a single time
 * rather than repeated per container. Only TArray sources are supported: TSet and TMap
 * are expected to be converted to TArray by the caller before evaluating a formula.
 *
 * Every method inspects the target's FArrayProperty at invoke time, so the element type
 * is not baked into the declarations.
 */
class FDotNetEnumerableType : public FDotNetSurrogateType
{
protected:
	TSharedRef<IFormulaType> const ElementType;

public:
	FDotNetEnumerableType(UClass* SurrogateClass, FProperty* LiteralProperty, const TSharedRef<IFormulaType>& ElementType)
		: FDotNetSurrogateType(SurrogateClass, LiteralProperty), ElementType(ElementType)
	{
	}

	virtual TSharedPtr<IFormulaType> GetUnderlyingType() const override { return this->ElementType; }

protected:
	virtual void InitializeFunctions(UClass* DeclaringClass, const TSharedRef<TMap<FString, FFormulaFunction>> FunctionList) override;
	virtual void InitializeFunctionNames(UClass* DeclaringClass, const TSharedRef<TArray<FString>> NameList) override;
};
