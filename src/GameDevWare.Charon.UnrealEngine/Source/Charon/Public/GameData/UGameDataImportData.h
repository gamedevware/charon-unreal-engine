// Copyright GameDevWare, Denis Zykov 2024

#pragma once

#include "EditorFramework/AssetImportData.h"
#include "UGameDataImportData.generated.h"

UCLASS(EditInlineNew)
class CHARON_API UGameDataImportData : public UAssetImportData
{
	GENERATED_BODY()

public:
#if WITH_EDITORONLY_DATA
	/*
	 * Address of game data server to which this asset is connected. 
	 */
	UPROPERTY(BlueprintReadOnly, Category="Game Data")
	FString ServerAddress;
	/*
	 * Id of project to which this asset is connected. 
	 */
	UPROPERTY(BlueprintReadOnly, Category="Game Data")
	FString ProjectId;
	/*
	 * Id of branch on project to which this asset is connected. 
	 */
	UPROPERTY(BlueprintReadOnly, Category="Game Data")
	FString BranchId;
	/*
	 * Name of project to which this asset is connected. Used for UI. 
	 */
	UPROPERTY(BlueprintReadOnly, Category="Game Data")
	FString ProjectName;
	/*
	 * Name of branch on project to which this asset is connected. Used for UI.
	 */
	UPROPERTY(BlueprintReadOnly, Category="Game Data")
	FString BranchName;
#endif
	
	/*
	 * Is this game data asset is connected to remote project. 
	 */
	bool IsConnected() const;

	/*
	 * Disconnect this data from remote project.
	 */
	UFUNCTION(BlueprintCallable, Category="Game Data")
	void Disconnect();

	/*
	 * Get Path to source Game Data file.
	 */
	FString GetNormalizedGameDataPath() const;
};