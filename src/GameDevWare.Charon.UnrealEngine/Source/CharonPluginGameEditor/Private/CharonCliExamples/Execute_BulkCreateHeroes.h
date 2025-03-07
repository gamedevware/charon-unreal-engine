// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "GameData/UGameDataBase.h"
#include "GameData/CommandLine/FCharonCli.h"
#include "GameDataExtensionCommandsLog.h"

static TSharedPtr<ICharonTask> CurrentBulkCreateTask;

/**
 * Example of creating multiple documents in a Game Data file.
 */
static void Execute_BulkCreateHeroes(const TArray<UObject*> ContextSensitiveObjects)
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
	UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Creating multiple heroes in '%s'..."), *GameDataPath);
	
	const TSharedRef<FJsonObject> ImportData = MakeShared<FJsonObject>();
	const TSharedRef<FJsonObject> Collections = MakeShared<FJsonObject>();
	TArray<TSharedPtr<FJsonValue>> HeroCollection = TArray<TSharedPtr<FJsonValue>>();

	TSharedPtr<FJsonObject> HeroDocument1 = MakeShared<FJsonObject>();
	HeroDocument1->SetStringField(TEXT("Id"), TEXT("SuperBoy"));
	HeroDocument1->SetStringField(TEXT("Name"), TEXT("Super Boy"));
	HeroDocument1->SetBoolField(TEXT("Religious"), false);

	TSharedPtr<FJsonObject> HeroDocument2 = MakeShared<FJsonObject>();
	HeroDocument2->SetStringField(TEXT("Id"), TEXT("WonderBobby"));
	HeroDocument2->SetStringField(TEXT("Name"), TEXT("Wonder Bobby"));
	HeroDocument2->SetBoolField(TEXT("Religious"), true);

	// Adding documents to the Hero collection
	HeroCollection.Add(MakeShared<FJsonValueObject>(HeroDocument1));
	HeroCollection.Add(MakeShared<FJsonValueObject>(HeroDocument2));

	// Adding the Hero collection to Collections
	Collections->SetArrayField("Hero", HeroCollection);

	// Adding Collections to the root node so Hero documents will be under /Collections/Hero/0 and /Collections/Hero/1 paths
	ImportData->SetObjectField("Collections", Collections);

	//
	// Documentation for the Import command and its parameters:
	// https://gamedevware.github.io/charon/advanced/commands/data_import.html
	//
	const auto BulkCreateTask = FCharonCli::Import(
		GameDataPath,
		FString(), // API Key
		TArray<FString> { TEXT("*") }, // Schemas
		ImportData,
		EImportMode::CreateAndUpdate
	);
	
	BulkCreateTask->OnCommandSucceed().AddLambda([](int _)
	{
		UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Successfully created documents."));
	});
	
	BulkCreateTask->OnCommandFailed().AddLambda([](const int ExitCode, const FString& Output)
	{
		UE_LOG(LogFGameDataExtensionCommands, Warning, TEXT("Command execution failed with exit code %d. Output: %s."), ExitCode, *Output);
	});
	
	BulkCreateTask->Start(/* EventDispatchThread */ ENamedThreads::GameThread);

	//
	// Ensure the task outlives this method call by storing it in a shared pointer.
	//
	CurrentBulkCreateTask = BulkCreateTask;
}
