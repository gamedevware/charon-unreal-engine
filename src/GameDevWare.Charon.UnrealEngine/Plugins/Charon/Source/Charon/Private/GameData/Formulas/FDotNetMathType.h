// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "FDotNetSurrogateType.h"
#include "GameData/Formulas/DotNetTypes/UDotNetArray.h"

class FDotNetMathType : public FDotNetSurrogateType
{
public:
	explicit FDotNetMathType(UClass* SurrogateClass)
		: FDotNetSurrogateType(SurrogateClass, nullptr)
	{
	}

protected:
	virtual void InitializeStaticFunctions(UClass* DeclaringClass, const TSharedRef<TMap<FString, FFormulaFunction>> FunctionList) override;
	virtual void InitializeStaticFunctionNames(UClass* DeclaringClass, const TSharedRef<TArray<FString>> NameList) override;
	
	static FFormulaFunction GetAbsFunction(UField* DeclaringType);
	static FFormulaFunction GetClampFunction(UField* DeclaringType);
	static FFormulaFunction GetDivRemFunction(UField* DeclaringType);
	static FFormulaFunction GetMaxFunction(UField* DeclaringType);
	static FFormulaFunction GetMinFunction(UField* DeclaringType);
	static FFormulaFunction GetRoundFunction(UField* DeclaringType);
	static FFormulaFunction GetSignFunction(UField* DeclaringType);
};
