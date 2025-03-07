#pragma once
#include "GameData/UGameDataBase.h"
#include "GameData/CommandLine/FCharonCli.h"
#include "GameDataExtensionCommandsLog.h"

static TSharedPtr<ICharonTask> CurrentExportTask;

/**
 * Example of exporting documents from Game Data file into list FJsonObject objects.
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
		
	UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Exporting heroes to FJsonObject from '%s'..."), *GameDataPath);

	//
	// Documentation for Export command and its parameters:
	// https://gamedevware.github.io/charon/advanced/commands/data_export.html
	//
	const auto ExportTask = FCharonCli::Export(
		GameDataPath,
		FString(), // Api Key
		TArray<FString> { TEXT("Hero") }, // Schemas
		TArray<FString>(), // Properties
		TArray<FString>(), // Languages
		EExportMode::Normal
	);
		
	ExportTask->OnCommandSucceed().AddLambda([](const TSharedPtr<FJsonObject>& Result)
	{
		const auto Collections = Result->GetObjectField(TEXT("Collections"));
		const auto HeroCollection = Collections->GetArrayField(TEXT("Hero"));
		for (const auto HeroValue : HeroCollection)
		{
			const auto HeroDocument = HeroValue->AsObject();
			const auto HeroName = HeroDocument->GetObjectField(TEXT("Name"));
			auto HeroNameEn = HeroName->GetStringField(TEXT("en-US")); 

			UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Found hero in game data: %s"), *HeroNameEn);
		}
	});
	ExportTask->OnCommandFailed().AddLambda([](const int ExitCode, const FString& Output)
	{
		UE_LOG(LogFGameDataExtensionCommands, Warning, TEXT("Command run failed with exit code %d. Output %s."), ExitCode, *Output);
	});
		
	ExportTask->Start(/* EventDispatchThread */ ENamedThreads::GameThread);

	//
	// Make sure that Task will outlive this method call.
	//
	CurrentExportTask = ExportTask;
}