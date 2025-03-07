#pragma once
#include "GameData/UGameDataBase.h"
#include "GameData/CommandLine/FCharonCli.h"
#include "GameDataExtensionCommandsLog.h"

static TSharedPtr<ICharonTask> CurrentValidateTask;

/**
* Example validating Game Data file.
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
	// Documentation for Validate command and its parameters:
	// https://gamedevware.github.io/charon/advanced/commands/data_validate.html
	//
	const auto ValidationOptions = TArray({ EValidationOption::CheckRequirements, EValidationOption::CheckTranslation });
	const auto ValidateTask = FCharonCli::Validate(
		GameDataPath,
		FString(), // Api Key
		ValidationOptions
	);
	
	ValidateTask->OnCommandSucceed().AddLambda([](FValidationReport ValidationReport)
	{
		TArray<FString> Issues;
		for (auto ValidationRecord : ValidationReport.Records)
		{
			for (auto ValidationError : ValidationRecord.Errors)
			{	
				Issues.Add(FString::Format(TEXT("{0}[id: {1}] {2}: {3}"), { ValidationRecord.SchemaName, ValidationRecord.Id, ValidationError.Code, ValidationError.Message }));
			}
		}

		if (Issues.IsEmpty())
		{
			UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Validation is finished. No issues."));
		}
		else
		{
			UE_LOG(LogFGameDataExtensionCommands, Warning, TEXT("Validation is finished. Issues[%d]: %s"), Issues.Num(), *FString::Join(Issues, TEXT("\r\n")));
		}
	});
	
	ValidateTask->OnCommandFailed().AddLambda([](const int ExitCode, const FString& Output)
	{
		UE_LOG(LogFGameDataExtensionCommands, Warning, TEXT("Command run failed with exit code %d. Output %s."), ExitCode, *Output);
	});
	
	ValidateTask->Start(/* EventDispatchThread */ ENamedThreads::GameThread);

	//
	// Make sure that Task will outlive this method call.
	//
	CurrentValidateTask = ValidateTask;
}