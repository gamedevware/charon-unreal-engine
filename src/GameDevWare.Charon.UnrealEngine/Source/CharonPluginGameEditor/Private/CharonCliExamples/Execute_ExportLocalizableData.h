#pragma once
#include "GameData/UGameDataBase.h"
#include "GameData/CommandLine/FCharonCli.h"
#include "GameDataExtensionCommandsLog.h"

static TSharedPtr<ICharonTask> CurrentI18NExportTask;

/**
 * Example exporting of localizable data from Game Data file into XLSX file.
 */
static void Execute_ExportLocalizableData(const TArray<UObject*> ContextSensitiveObjects)
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
		
	UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Exporting localizable data from '%s'..."), *GameDataPath);

	const FString XslxExportPath = FPaths::CreateTempFilename(FPlatformProcess::UserTempDir(), TEXT("CharonExportToFile"), TEXT(".xslx"));

	//
	// Documentation for I18NExport command and its parameters:
	// https://gamedevware.github.io/charon/advanced/commands/data_i18n_export.html
	//
	const auto ExportTask = FCharonCli::I18NExportToFile(
		GameDataPath,
		FString(), // Api Key
		TArray<FString> { TEXT("*") }, // Schemas
		TEXT("en-US"), // Source Language
		TEXT("es-ES"), // Target Language
		XslxExportPath,
		TEXT("xslx") // Format
	);
		
	ExportTask->OnCommandSucceed().AddLambda([XslxExportPath](int _)
	{
		UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Export succeed into file: %s"), *XslxExportPath);
	});
		
	ExportTask->OnCommandFailed().AddLambda([](const int ExitCode, const FString& Output)
	{
		UE_LOG(LogFGameDataExtensionCommands, Warning, TEXT("Command run failed with exit code %d. Output %s."), ExitCode, *Output);
	});
		
	ExportTask->Start(/* EventDispatchThread */ ENamedThreads::GameThread);

	//
	// Make sure that Task will outlive this method call.
	//
	CurrentI18NExportTask = ExportTask;
}