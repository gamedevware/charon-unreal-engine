// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "GameData/UGameDataBase.h"
#include "GameData/CommandLine/FCharonCli.h"
#include "GameDataExtensionCommandsLog.h"

static TSharedPtr<ICharonTask> CurrentI18NExportTask;

/**
 * Example of exporting localizable data from a Game Data file into an XLSX file.
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

	// Generate a temporary file path for the exported XLSX file.
	const FString XlsxExportPath = FPaths::CreateTempFilename(FPlatformProcess::UserTempDir(), TEXT("CharonExport"), TEXT(".xlsx"));

	//
	// Documentation for the I18NExport command and its parameters:
	// https://gamedevware.github.io/charon/advanced/commands/data_i18n_export.html
	//
	const auto ExportTask = FCharonCli::I18NExportToFile(
		GameDataPath,
		FString(), // API Key
		TArray<FString>{ TEXT("*") }, // Schemas (use "*" to export all)
		TEXT("en-US"), // Source language
		TEXT("es-ES"), // Target language
		XlsxExportPath, // Output file path
		TEXT("xlsx") // Export format
	);
		
	ExportTask->OnCommandSucceed().AddLambda([XlsxExportPath](int _)
	{
		UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Export succeeded. File saved to: %s"), *XlsxExportPath);
	});
		
	ExportTask->OnCommandFailed().AddLambda([](const int ExitCode, const FString& Output)
	{
		UE_LOG(LogFGameDataExtensionCommands, Warning, TEXT("Command execution failed with exit code %d. Output: %s"), ExitCode, *Output);
	});
		
	ExportTask->Start(/* EventDispatchThread */ ENamedThreads::GameThread);

	//
	// Ensure the task outlives this method call by storing it in a shared pointer.
	//
	CurrentI18NExportTask = ExportTask;
}
