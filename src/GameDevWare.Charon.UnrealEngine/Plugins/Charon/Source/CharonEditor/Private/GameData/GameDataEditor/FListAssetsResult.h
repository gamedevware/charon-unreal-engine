#pragma once

#include "CoreMinimal.h"
#include "FGameAsset.h"
#include "Engine/EngineTypes.h" // For FString, bool, etc.
#include "FListAssetsResult.generated.h" // Must be included for USTRUCT()

// Equivalent to TypeScript's ListAssetsResult
USTRUCT(BlueprintType)
struct FListAssetsResult
{
	GENERATED_BODY()

public:
	// Array of assets (maps to TypeScript's ReadonlyArray<GameAsset>)
	UPROPERTY(BlueprintReadOnly, Category = "WebBrowser|ListAssetsResult")
	TArray<FGameAsset> Assets;

	// Total number of assets (maps to TypeScript's `total: number`)
	UPROPERTY(BlueprintReadOnly, Category = "WebBrowser|ListAssetsResult")
	int32 Total;

	// Optional: Constructor for easy initialization in C++
	FListAssetsResult() : Total(0) {}
};