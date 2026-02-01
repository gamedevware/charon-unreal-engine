// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "UFormulaExpressionDefaultGlobal.generated.h"

UCLASS(Hidden)
class CHARON_API UFormulaExpressionDefaultGlobal : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, Category = "Scoped")
	TObjectPtr<UObject> This;
	
	UPROPERTY(VisibleAnywhere, Category = "Scoped")
	TObjectPtr<UObject> GameData;
};
