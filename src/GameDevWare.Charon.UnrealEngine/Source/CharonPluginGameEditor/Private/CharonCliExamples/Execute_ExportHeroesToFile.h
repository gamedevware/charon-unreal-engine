// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "GameData/UGameDataBase.h"
#include "GameData/CommandLine/FCharonCli.h"
#include "GameDataExtensionCommandsLog.h"
#include "TryReadJson.h"

static TSharedPtr<ICharonTask> CurrentExportToFileTask;

/**
 * Example of exporting hero documents from a Game Data file into an external file.
 */
static void Execute_ExportHeroesToFile(const TArray<UObject*> ContextSensitiveObjects)
{
	if (ContextSensitiveObjects.IsEmpty())
	{
		return;
	}
	
	const UGameDataBase* GameDataPtr = Cast<UGameDataBase>(ContextSensitiveObjects[0]);
	if (!GameDataPtr)
	{
		return;
	}
	
	const FString GameDataPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir(), GameDataPtr->AssetImportData->GetFirstFilename());
	
	UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Exporting hero documents to file from '%s'..."), *GameDataPath);

	// Generate a temporary file path for the export.
	const FString ExportPath = FPaths::CreateTempFilename(FPlatformProcess::UserTempDir(), TEXT("CharonExport"));

	//
	// Documentation for the Export command and its parameters:
	// https://gamedevware.github.io/charon/advanced/commands/data_export.html
	//
	const auto ExportTask = FCharonCli::ExportToFile(
		GameDataPath,
		FString(), // API Key
		TArray<FString>{ TEXT("Hero") }, // Schemas to export
		TArray<FString>(), // Properties (empty = all)
		TArray<FString>(), // Languages (empty = all)
		EExportMode::Normal,
		ExportPath, // Output file path
		TEXT("json") // Export format
	);
	
	ExportTask->OnCommandSucceed().AddLambda([ExportPath](int _)
	{
		TSharedPtr<FJsonObject> ExportJsonObject;
		if (!TryReadJsonFile(ExportPath, ExportJsonObject))
		{
			UE_LOG(LogFGameDataExtensionCommands, Warning, TEXT("Failed to read exported JSON file: %s"), *ExportPath);
			return;
		}
		
		const auto Collections = ExportJsonObject->GetObjectField(TEXT("Collections"));
		const auto HeroCollection = Collections->GetArrayField(TEXT("Hero"));
		for (const auto& HeroValue : HeroCollection)
		{
			const auto HeroObject = HeroValue->AsObject();
			const auto HeroName = HeroObject->GetObjectField(TEXT("Name"));
			const auto HeroNameEn = HeroName->GetStringField(TEXT("en-US")); 

			UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Found hero in exported data: %s"), *HeroNameEn);
		}
	});
	
	ExportTask->OnCommandFailed().AddLambda([](const int ExitCode, const FString& Output)
	{
		UE_LOG(LogFGameDataExtensionCommands, Warning, TEXT("Command execution failed with exit code %d. Output: %s."), ExitCode, *Output);
	});
	
	ExportTask->Start(/* EventDispatchThread */ ENamedThreads::GameThread);

	//
	// Ensure the task outlives this method call by storing it in a shared pointer.
	//
	CurrentExportToFileTask = ExportTask;
}
