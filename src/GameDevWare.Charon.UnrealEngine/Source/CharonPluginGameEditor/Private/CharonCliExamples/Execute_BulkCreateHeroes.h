#pragma once
#include "GameData/UGameDataBase.h"
#include "GameData/CommandLine/FCharonCli.h"
#include "GameDataExtensionCommandsLog.h"

static TSharedPtr<ICharonTask> CurrentBulkCreateTask;

/**
 * Example creating multiple documents in Game Data file.
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
	UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Creating multiple heroes to '%s'..."), *GameDataPath);
	
	const TSharedRef<FJsonObject> ImportData = MakeShared<FJsonObject>();
	const TSharedRef<FJsonObject> Collections = MakeShared<FJsonObject>();
	TArray<TSharedPtr<FJsonValue>> HeroCollection = TArray<TSharedPtr<FJsonValue>>();

	TSharedPtr<FJsonObject> HeroDocument1 = MakeShared<FJsonObject>();
	HeroDocument1->SetStringField(TEXT("Id"), TEXT("SuperBoy"));
	HeroDocument1->SetStringField(TEXT("Name"), TEXT("Super Boy"));
	HeroDocument1->SetBoolField(TEXT("Religious"), false);

	TSharedPtr<FJsonObject> HeroDocument2 = MakeShared<FJsonObject>();
	HeroDocument2->SetStringField(TEXT("Id"), TEXT("WounderBobby"));
	HeroDocument2->SetStringField(TEXT("Name"), TEXT("Wounder Bobby"));
	HeroDocument2->SetBoolField(TEXT("Religious"), true);

	// Putting documents into Hero collection
	HeroCollection.Add(MakeShared<FJsonValueObject>(HeroDocument1));
	HeroCollection.Add(MakeShared<FJsonValueObject>(HeroDocument2));

	// Putting Hero collection inside Collections
	Collections->SetArrayField("Hero", HeroCollection);

	// Putting Collection into root no, so Hero documents would be under /Collections/Hero/0 and /Collections/Hero/1 paths
	ImportData->SetObjectField("Collections", Collections);

	//
	// Documentation for Import command and its parameters:
	// https://gamedevware.github.io/charon/advanced/commands/data_import.html
	//
	const auto BulkCreateTask = FCharonCli::Import(
		GameDataPath,
		FString(), // Api Key
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
		UE_LOG(LogFGameDataExtensionCommands, Warning, TEXT("Command run failed with exit code %d. Output %s."), ExitCode, *Output);
	});
	
	BulkCreateTask->Start(/* EventDispatchThread */ ENamedThreads::GameThread);

	//
	// Make sure that Task will outlive this method call.
	//
	CurrentBulkCreateTask = BulkCreateTask;
}