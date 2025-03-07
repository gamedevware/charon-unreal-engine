// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "GameData/UGameDataBase.h"
#include "GameData/CommandLine/FCharonCli.h"
#include "GameDataExtensionCommandsLog.h"

static TSharedPtr<ICharonTask> CurrentBulkDeleteTask;

/**
 * Example of deleting multiple documents from a Game Data file.
 */
static void Execute_BulkDeleteHeroes(const TArray<UObject*> ContextSensitiveObjects)
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
	UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Deleting multiple heroes from '%s'..."), *GameDataPath);
	
	const TSharedRef<FJsonObject> ImportData = MakeShared<FJsonObject>();
	const TSharedRef<FJsonObject> Collections = MakeShared<FJsonObject>();
	TArray<TSharedPtr<FJsonValue>> HeroCollection = TArray<TSharedPtr<FJsonValue>>();

	// Only IDs are required for mass deletion
	TSharedPtr<FJsonObject> HeroDocument1 = MakeShared<FJsonObject>();
	HeroDocument1->SetStringField(TEXT("Id"), TEXT("Crossbower"));

	TSharedPtr<FJsonObject> HeroDocument2 = MakeShared<FJsonObject>();
	HeroDocument2->SetStringField(TEXT("Id"), TEXT("Monstrosity"));

	TSharedPtr<FJsonObject> HeroDocument3 = MakeShared<FJsonObject>();
	HeroDocument3->SetStringField(TEXT("Id"), TEXT("Zealot"));
	
	// Adding documents to the Hero collection
	HeroCollection.Add(MakeShared<FJsonValueObject>(HeroDocument1));
	HeroCollection.Add(MakeShared<FJsonValueObject>(HeroDocument2));
	HeroCollection.Add(MakeShared<FJsonValueObject>(HeroDocument3));

	// Adding the Hero collection to Collections
	Collections->SetArrayField("Hero", HeroCollection);

	// Adding Collections to the root node so Hero documents will be under /Collections/Hero/0, /Collections/Hero/1, etc.
	ImportData->SetObjectField("Collections", Collections);

	//
	// Documentation for the Import command and its parameters:
	// https://gamedevware.github.io/charon/advanced/commands/data_import.html
	//
	const auto BulkDeleteTask = FCharonCli::Import(
		GameDataPath,
		FString(), // API Key
		TArray<FString> { TEXT("*") }, // Schemas
		ImportData,
		EImportMode::Delete
	);
	
	BulkDeleteTask->OnCommandSucceed().AddLambda([](int _)
	{
		UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Successfully deleted documents."));
	});
	
	BulkDeleteTask->OnCommandFailed().AddLambda([](const int ExitCode, const FString& Output)
	{
		UE_LOG(LogFGameDataExtensionCommands, Warning, TEXT("Command execution failed with exit code %d. Output: %s."), ExitCode, *Output);
	});
	
	BulkDeleteTask->Start(/* EventDispatchThread */ ENamedThreads::GameThread);

	//
	// Ensure the task outlives this method call by storing it in a shared pointer.
	//
	CurrentBulkDeleteTask = BulkDeleteTask;
}
