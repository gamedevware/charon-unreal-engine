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
	 * Name of game data class. By default, it is source file name and "U" prefix.
	 */
	UPROPERTY(BlueprintReadOnly, Category="Game Data")
	FString GameDataClassName;
	/*
	 * Name of document class. By default, it is a "UDocument".
	 */
	UPROPERTY(BlueprintReadOnly, Category="Game Data")
	FString GameDataDocumentClassName;
	/*
	 * List of pre-processor defines which will be defined in each source file. Separated by comma.
	 */
	UPROPERTY(BlueprintReadOnly, Category="Game Data")
	FString DefineConstants;
	/*
	 * List of languages to used to import into .uasset file during import/synchronization.
	 */
	UPROPERTY(BlueprintReadOnly, Category="Game Data")
	TArray<FString> PublishLanguages;
	/*
	 * Source code optimizations enabled for generated source code.
	 */
	UPROPERTY(BlueprintReadOnly, Category="Game Data")
	int Optimizations;
	/*
	 * Source code indentation.
	 */
	UPROPERTY(BlueprintReadOnly, Category="Game Data")
	int Indentation;
	/*
	 * Source code line ending style.
	 */
	UPROPERTY(BlueprintReadOnly, Category="Game Data")
	int LineEnding;
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