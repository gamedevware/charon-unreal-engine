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
	inline bool IsConnected() const
	{
#if WITH_EDITORONLY_DATA
		return !ServerAddress.IsEmpty() && !ProjectId.IsEmpty() && !BranchId.IsEmpty();
#else
		return false;
#endif
	}

	/*
	 * Disconnect this data from remote project.
	 */
	UFUNCTION(BlueprintCallable, Category="Game Data")
	inline void Disconnect()
	{
#if WITH_EDITORONLY_DATA
		ServerAddress.Empty();
		ProjectId.Empty();
		ProjectName.Empty();
		BranchId.Empty();
		BranchName.Empty();
#endif
	}

	inline FString GetNormalizedGameDataPath() const
	{
		FString GameDataFilePath = GetFirstFilename();
		if (GameDataFilePath.IsEmpty())
		{
			return GameDataFilePath;
		}
		GameDataFilePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir(), GameDataFilePath);
		FPaths::NormalizeFilename(GameDataFilePath);
		FPaths::CollapseRelativeDirectories(GameDataFilePath);
		return GameDataFilePath;
	}
};