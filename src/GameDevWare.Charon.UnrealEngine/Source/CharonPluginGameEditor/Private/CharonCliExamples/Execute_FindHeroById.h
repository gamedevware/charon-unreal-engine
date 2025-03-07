// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "GameData/UGameDataBase.h"
#include "GameData/CommandLine/FCharonCli.h"
#include "GameDataExtensionCommandsLog.h"

static TSharedPtr<ICharonTask> CurrentFindTask;

/**
 * Example of retrieving a single document by ID from the Game Data file.
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
	UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Searching for hero document in '%s'..."), *GameDataPath);
	
	//
	// Documentation for the FindDocument command and its parameters:
	// https://gamedevware.github.io/charon/advanced/commands/data_find.html
	//
	const auto FindTask = FCharonCli::FindDocument(
		GameDataPath,
		FString(), // API Key
		TEXT("Hero"), // Schema
		TEXT("Crossbower") // Document ID
	);
	
	FindTask->OnCommandSucceed().AddLambda([](TSharedPtr<FJsonObject> FoundDocument)
	{
		UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Successfully found document with ID: %s"), *FoundDocument->GetStringField(TEXT("Id")));
	});
	
	FindTask->OnCommandFailed().AddLambda([](const int ExitCode, const FString& Output)
	{
		UE_LOG(LogFGameDataExtensionCommands, Warning, TEXT("Command execution failed with exit code %d. Output: %s"), ExitCode, *Output);
	});
	
	FindTask->Start(/* EventDispatchThread */ ENamedThreads::GameThread);

	//
	// Ensure the task outlives this method call by storing it in a shared pointer.
	//
	CurrentFindTask = FindTask;
}
