// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "GameData/UGameDataBase.h"
#include "GameData/CommandLine/FCharonCli.h"
#include "GameDataExtensionCommandsLog.h"

static TSharedPtr<ICharonTask> CurrentValidateTask;

/**
 * Example of validating a Game Data file.
 */
static void Execute_ValidateGameData(const TArray<UObject*> ContextSensitiveObjects)
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
	UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Validating game data at '%s'..."), *GameDataPath);
	
	//
	// Documentation for the Validate command and its parameters:
	// https://gamedevware.github.io/charon/advanced/commands/data_validate.html
	//
	const auto ValidationOptions = TArray({ EValidationOption::CheckRequirements, EValidationOption::CheckTranslation });
	const auto ValidateTask = FCharonCli::Validate(
		GameDataPath,
		FString(), // API Key
		ValidationOptions
	);
	
	ValidateTask->OnCommandSucceed().AddLambda([](FValidationReport ValidationReport)
	{
		TArray<FString> Issues;
		for (const auto& ValidationRecord : ValidationReport.Records)
		{
			for (const auto& ValidationError : ValidationRecord.Errors)
			{	
				Issues.Add(FString::Format(TEXT("{0}[id: {1}] {2}: {3}"), { ValidationRecord.SchemaName, ValidationRecord.Id, ValidationError.Code, ValidationError.Message }));
			}
		}

		if (Issues.IsEmpty())
		{
			UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Validation completed. No issues found."));
		}
		else
		{
			UE_LOG(LogFGameDataExtensionCommands, Warning, TEXT("Validation completed. Found %d issues:\n%s"), Issues.Num(), *FString::Join(Issues, TEXT("\n")));
		}
	});
	
	ValidateTask->OnCommandFailed().AddLambda([](const int ExitCode, const FString& Output)
	{
		UE_LOG(LogFGameDataExtensionCommands, Warning, TEXT("Command execution failed with exit code %d. Output:\n%s"), ExitCode, *Output);
	});
	
	ValidateTask->Start(/* EventDispatchThread */ ENamedThreads::GameThread);

	//
	// Ensure the task outlives this method call by storing it in a shared pointer.
	//
	CurrentValidateTask = ValidateTask;
}