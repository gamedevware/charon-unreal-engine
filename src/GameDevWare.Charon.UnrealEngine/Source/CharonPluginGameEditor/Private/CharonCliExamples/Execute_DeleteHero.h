// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "GameData/UGameDataBase.h"
#include "GameData/CommandLine/FCharonCli.h"
#include "GameDataExtensionCommandsLog.h"

static TSharedPtr<ICharonTask> CurrentDeleteTask;

/**
 * Example of deleting a single document in a Game Data file.
 */
static void Execute_DeleteHero(const TArray<UObject*> ContextSensitiveObjects)
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
	UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Deleting hero document at '%s'..."), *GameDataPath);

	const TSharedPtr<FJsonObject> HeroDocument = MakeShared<FJsonObject>();
	HeroDocument->SetStringField(TEXT("Id"), TEXT("SuperBoy"));
	
	//
	// Documentation for the DeleteDocument command and its parameters:
	// https://gamedevware.github.io/charon/advanced/commands/data_delete.html
	//
	const auto DeleteTask = FCharonCli::DeleteDocument(
		GameDataPath,
		FString(), // API Key
		TEXT("Hero"), // Schema
		HeroDocument.ToSharedRef()  // Document to delete
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
	CurrentDeleteTask = DeleteTask;
}
