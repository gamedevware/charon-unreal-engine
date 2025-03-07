#pragma once
#include "GameData/UGameDataBase.h"
#include "GameData/CommandLine/FCharonCli.h"
#include "GameDataExtensionCommandsLog.h"
#include "TryReadJson.h"

static TSharedPtr<ICharonTask> CurrentExportToFileTask;

/**
 * Example of exporting documents from Game Data file into another file.
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
	
	UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Exporting heroes to FJsonObject from '%s'..."), *GameDataPath);

	const FString ExportPath = FPaths::CreateTempFilename(FPlatformProcess::UserTempDir(), TEXT("CharonExportToFile"));
	//
	// Documentation for Export command and its parameters:
	// https://gamedevware.github.io/charon/advanced/commands/data_export.html
	//
	const auto ExportTask = FCharonCli::ExportToFile(
		GameDataPath,
		FString(), // Api Key
		TArray<FString> { TEXT("Hero") }, // Schemas
		TArray<FString>(), // Properties
		TArray<FString>(), // Languages
		EExportMode::Normal,
		ExportPath,
		TEXT("json") // Format
	);
	
	ExportTask->OnCommandSucceed().AddLambda([ExportPath](int _)
	{
		TSharedPtr<FJsonObject> ExportJsonObject;
		if (!TryReadJsonFile(ExportPath, ExportJsonObject))
		{
			return;
		}
		
		const auto Collections = ExportJsonObject->GetObjectField(TEXT("Collections"));
		const auto HeroCollection = Collections->GetArrayField(TEXT("Hero"));
		for (const auto HeroValue : HeroCollection)
		{
			const auto HeroObject = HeroValue->AsObject();
			const auto HeroName = HeroObject->GetObjectField(TEXT("Name"));
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
	CurrentExportToFileTask = ExportTask;
}