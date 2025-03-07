// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "GameData/UGameDataBase.h"
#include "GameData/CommandLine/FCharonCli.h"
#include "GameDataExtensionCommandsLog.h"

static TSharedPtr<ICharonTask> CurrentExportTask;

/**
 * Example of exporting hero documents from a Game Data file into a list of FJsonObject objects.
 */
static void Execute_ExportHeroes(const TArray<UObject*> ContextSensitiveObjects)
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
		
	UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Exporting hero documents to FJsonObject from '%s'..."), *GameDataPath);

	//
	// Documentation for the Export command and its parameters:
	// https://gamedevware.github.io/charon/advanced/commands/data_export.html
	//
	const auto ExportTask = FCharonCli::Export(
		GameDataPath,
		FString(), // API Key
		TArray<FString>{ TEXT("Hero") }, // Schemas to export
		TArray<FString>(), // Properties (empty = all)
		TArray<FString>(), // Languages (empty = all)
		EExportMode::Normal
	);
		
	ExportTask->OnCommandSucceed().AddLambda([](const TSharedPtr<FJsonObject>& Result)
	{
		const auto Collections = Result->GetObjectField(TEXT("Collections"));
		const auto HeroCollection = Collections->GetArrayField(TEXT("Hero"));
		for (const auto& HeroValue : HeroCollection)
		{
			const auto HeroDocument = HeroValue->AsObject();
			const auto HeroName = HeroDocument->GetObjectField(TEXT("Name"));
			const auto HeroNameEn = HeroName->GetStringField(TEXT("en-US")); 

			UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Found hero in game data: %s"), *HeroNameEn);
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
	CurrentExportTask = ExportTask;
}
