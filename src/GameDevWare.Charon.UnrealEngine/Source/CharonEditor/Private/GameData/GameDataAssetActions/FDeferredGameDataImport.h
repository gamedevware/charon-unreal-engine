#pragma once

#include "FDeferredGameDataImport.generated.h"

USTRUCT()
struct FDeferredGameDataImport
{
	GENERATED_BODY();
public:
	/** File name of game data. */
	UPROPERTY()
	FString FileName;
	
	/** Class name of game data. */
	UPROPERTY()
	FString ClassName;
};
