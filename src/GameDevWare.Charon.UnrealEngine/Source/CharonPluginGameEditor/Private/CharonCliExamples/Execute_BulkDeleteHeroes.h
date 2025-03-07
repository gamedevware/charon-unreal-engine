#pragma once
#include "GameData/UGameDataBase.h"
#include "GameData/CommandLine/FCharonCli.h"
#include "GameDataExtensionCommandsLog.h"

static TSharedPtr<ICharonTask> CurrentBulkDeleteTask;

/**
 * Example creating multiple documents in Game Data file.
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
	UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Deleting multiple heroes to '%s'..."), *GameDataPath);
	
	const TSharedRef<FJsonObject> ImportData = MakeShared<FJsonObject>();
	const TSharedRef<FJsonObject> Collections = MakeShared<FJsonObject>();
	TArray<TSharedPtr<FJsonValue>> HeroCollection = TArray<TSharedPtr<FJsonValue>>();

	// only Ids are required for mass delete
	TSharedPtr<FJsonObject> HeroDocument1 = MakeShared<FJsonObject>();
	HeroDocument1->SetStringField(TEXT("Id"), TEXT("Crossbower"));

	TSharedPtr<FJsonObject> HeroDocument2 = MakeShared<FJsonObject>();
	HeroDocument2->SetStringField(TEXT("Id"), TEXT("Monstrosity"));

	TSharedPtr<FJsonObject> HeroDocument3 = MakeShared<FJsonObject>();
	HeroDocument3->SetStringField(TEXT("Id"), TEXT("Zealot"));
	
	// Putting documents into Hero collection
	HeroCollection.Add(MakeShared<FJsonValueObject>(HeroDocument1));
	HeroCollection.Add(MakeShared<FJsonValueObject>(HeroDocument2));
	HeroCollection.Add(MakeShared<FJsonValueObject>(HeroDocument3));

	// Putting Hero collection inside Collections
	Collections->SetArrayField("Hero", HeroCollection);

	// Putting Collection into root no, so Hero documents would be under /Collections/Hero/0 and /Collections/Hero/1 paths
	ImportData->SetObjectField("Collections", Collections);

	//
	// Documentation for Import command and its parameters:
	// https://gamedevware.github.io/charon/advanced/commands/data_import.html
	//
	const auto BulkDeleteTask = FCharonCli::Import(
		GameDataPath,
		FString(), // Api Key
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
		UE_LOG(LogFGameDataExtensionCommands, Warning, TEXT("Command run failed with exit code %d. Output %s."), ExitCode, *Output);
	});
	
	BulkDeleteTask->Start(/* EventDispatchThread */ ENamedThreads::GameThread);

	//
	// Make sure that Task will outlive this method call.
	//
	CurrentBulkDeleteTask = BulkDeleteTask;
}