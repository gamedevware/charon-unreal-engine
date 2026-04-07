// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "FDotNetSurrogateType.h"
#include "GameData/Formulas/DotNetTypes/UDotNetArray.h"

class FDotNetStringType : public FDotNetSurrogateType
{
public:
	explicit FDotNetStringType(UClass* SurrogateClass, FProperty* LiteralProperty)
		: FDotNetSurrogateType(SurrogateClass, LiteralProperty)
	{
	}

protected:
	virtual void InitializeStaticFunctions(UClass* DeclaringClass, const TSharedRef<TMap<FString, FFormulaFunction>> FunctionList) override;
	virtual void InitializeStaticFunctionNames(UClass* DeclaringClass, const TSharedRef<TArray<FString>> NameList) override;
private:
	static FFormulaFunction GetConcatFunction(UField* DeclaringType);
	static FFormulaFunction GetFormatFunction(UField* DeclaringType);
	static FFormulaFunction GetJoinFunction(UField* DeclaringType);
};
