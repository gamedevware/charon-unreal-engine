// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "GameData/UGameDataBase.h"
#include "GameData/CommandLine/FCharonCli.h"
#include "GameDataExtensionCommandsLog.h"

static TSharedPtr<ICharonTask> CurrentDeleteByIdTask;

/**
 * Example of deleting a single document by ID in a Game Data file.
 */
static void Execute_DeleteHeroById(const TArray<UObject*> ContextSensitiveObjects)
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
	UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Deleting hero document with ID 'SuperBoy' from '%s'..."), *GameDataPath);
	
	//
	// Documentation for the DeleteDocument command and its parameters:
	// https://gamedevware.github.io/charon/advanced/commands/data_delete.html
	//
	const auto DeleteTask = FCharonCli::DeleteDocument(
		GameDataPath,
		FString(), // API Key
		TEXT("Hero"), // Schema
		TEXT("SuperBoy")  // Document ID
	);
	
	DeleteTask->OnCommandSucceed().AddLambda([](TSharedPtr<FJsonObject> DeletedDocument)
	{
		UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Successfully deleted document."));
	});
	
	DeleteTask->OnCommandFailed().AddLambda([](const int ExitCode, const FString& Output)
	{
		UE_LOG(LogFGameDataExtensionCommands, Warning, TEXT("Command execution failed with exit code %d. Output: %s."), ExitCode, *Output);
	});
	
	DeleteTask->Start(/* EventDispatchThread */ ENamedThreads::GameThread);

	//
	// Ensure the task outlives this method call by storing it in a shared pointer.
	//
	CurrentDeleteByIdTask = DeleteTask;
}
