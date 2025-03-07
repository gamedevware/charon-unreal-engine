// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "FDeferredGameDataImportRecord.generated.h"

USTRUCT()
struct FDeferredGameDataImportRecord
{
	GENERATED_BODY();
public:
	/** File name of game data. */
	UPROPERTY()
	FString FileName;
	/** Module name of game data. */
	UPROPERTY()
	FString ModuleName;
	/** Class name of game data. */
	UPROPERTY()
	FString ClassName;
};
