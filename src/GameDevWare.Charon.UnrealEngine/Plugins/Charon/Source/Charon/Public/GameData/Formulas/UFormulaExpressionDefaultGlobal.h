#pragma once

#include "UFormulaExpressionDefaultGlobal.generated.h"

UCLASS()
class CHARON_API UFormulaExpressionDefaultGlobal : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, Category = "Scoped", meta = (FormulaMemberName = "this"))
	TObjectPtr<UObject> ThisObject;
	
	UPROPERTY(VisibleAnywhere, Category = "Scoped")
	TObjectPtr<UObject> GameData;
};
