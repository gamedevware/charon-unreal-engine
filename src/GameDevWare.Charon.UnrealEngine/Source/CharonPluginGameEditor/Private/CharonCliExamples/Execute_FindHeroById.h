#pragma once
#include "GameData/UGameDataBase.h"
#include "GameData/CommandLine/FCharonCli.h"
#include "GameDataExtensionCommandsLog.h"

static TSharedPtr<ICharonTask> CurrentFindTask;

/**
* Example retrieving single document by id in Game Data file.
*/
static void Execute_FindHeroById(const TArray<UObject*> ContextSensitiveObjects)
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
	UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Finding hero document at '%s'..."), *GameDataPath);
	
	//
	// Documentation for FindDocument command and its parameters:
	// https://gamedevware.github.io/charon/advanced/commands/data_find.html
	//
	const auto FindTask = FCharonCli::FindDocument(
		GameDataPath,
		FString(), // Api Key
		TEXT("Hero"), // Schema
		TEXT("Crossbower") // Id
	);
	
	FindTask->OnCommandSucceed().AddLambda([](TSharedPtr<FJsonObject> FoundDocument)
	{
		UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Successfully found %s document."), *FoundDocument->GetStringField(TEXT("Id")));
	});
	
	FindTask->OnCommandFailed().AddLambda([](const int ExitCode, const FString& Output)
	{
		UE_LOG(LogFGameDataExtensionCommands, Warning, TEXT("Command run failed with exit code %d. Output %s."), ExitCode, *Output);
	});
	
	FindTask->Start(/* EventDispatchThread */ ENamedThreads::GameThread);

	//
	// Make sure that Task will outlive this method call.
	//
	CurrentFindTask = FindTask;
}