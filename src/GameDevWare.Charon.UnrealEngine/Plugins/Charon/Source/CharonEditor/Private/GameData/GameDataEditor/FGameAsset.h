#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h" // For FString, bool, etc.
#include "FGameAsset.generated.h" // Must be included for USTRUCT()

// Equivalent to TypeScript's GameAsset
USTRUCT(BlueprintType) // BlueprintType makes it usable in Blueprints
struct FGameAsset
{
	GENERATED_BODY()

public:
	// Name of the asset
	UPROPERTY(BlueprintReadOnly, Category = "WebBrowser|GameAsset")
	FString Name;

	// Whether the asset has a thumbnail
	UPROPERTY(BlueprintReadOnly, Category = "WebBrowser|GameAsset", meta = (JsonKeyName = "hasThumbnail"))
	bool HasThumbnail;

	// Hash of thumbnail. If available
	UPROPERTY(BlueprintReadOnly, Category = "WebBrowser|GameAsset", meta = (JsonKeyName = "thumbnailHash"))
	FString ThumbnailHash;
	
	// Type of the asset (e.g., "Texture", "Material")
	UPROPERTY(BlueprintReadOnly, Category = "WebBrowser|GameAsset")
	FString Type;
	
	// Path to the asset (e.g., "/Game/Textures/MyTexture")
	UPROPERTY(BlueprintReadOnly, Category = "WebBrowser|GameAsset")
	FString Path;

	// Optional: Constructor for easy initialization in C++
	FGameAsset() : HasThumbnail(false) {}
};