#pragma once
#include "GameData/UGameDataBase.h"
#include "GameData/CommandLine/FCharonCli.h"
#include "GameDataExtensionCommandsLog.h"

static TSharedPtr<ICharonTask> CurrentUpdateTask;

/**
* Example updating single document in Game Data file.
*/
static void Execute_UpdateHero(const TArray<UObject*> ContextSensitiveObjects)
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
	UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Updating hero document at '%s'..."), *GameDataPath);
	
	const TSharedPtr<FJsonObject> HeroDocument = MakeShared<FJsonObject>();
	// Id field is mandatory for the update operation.
	HeroDocument->SetStringField(TEXT("Id"), TEXT("SuperBoy"));
	
	// To update field just specify name and new value.
	HeroDocument->SetBoolField(TEXT("Religious"), true);
	
	// When updating collections, you must specify all existing documents in the collection by their ID.
	// Any missing documents will be deleted during the update operation.
	// Sometimes it's easier to find a document through a FindDocument() operation, update the required field, and UpdateDocument() the document back.
	TArray<TSharedPtr<FJsonValue>> Armors = TArray<TSharedPtr<FJsonValue>>();
	
	TSharedPtr<FJsonObject> Armor1 = MakeShared<FJsonObject>();
	Armor1->SetStringField(TEXT("Id"), TEXT("CrossbowerArmor1"));
	
	TSharedPtr<FJsonObject> Armor2 = MakeShared<FJsonObject>();
	Armor2->SetStringField(TEXT("Id"), TEXT("CrossbowerArmor2"));

	TSharedPtr<FJsonObject> Armor3 = MakeShared<FJsonObject>();
	Armor3->SetStringField(TEXT("Id"), TEXT("CrossbowerArmor3"));

	TSharedPtr<FJsonObject> Armor4 = MakeShared<FJsonObject>();
	Armor4->SetStringField(TEXT("Id"), TEXT("CrossbowerArmor4"));

	TSharedPtr<FJsonObject> Armor5 = MakeShared<FJsonObject>();
	Armor5->SetStringField(TEXT("Id"), TEXT("CrossbowerArmor5"));
	Armor5->SetNumberField(TEXT("HitPoints"), 10);
	Armor5->SetNumberField(TEXT("Dodge"), 50);
	
	Armors.Add(MakeShared<FJsonValueObject>(Armor1));
	Armors.Add(MakeShared<FJsonValueObject>(Armor2));
	Armors.Add(MakeShared<FJsonValueObject>(Armor3));
	Armors.Add(MakeShared<FJsonValueObject>(Armor4));
	Armors.Add(MakeShared<FJsonValueObject>(Armor5));
	HeroDocument->SetArrayField(TEXT("Armors"), Armors);

	//
	// Documentation for UpdateDocument command and its parameters:
	// https://gamedevware.github.io/charon/advanced/commands/data_update.html
	//
	const auto UpdateTask = FCharonCli::UpdateDocument(
		GameDataPath,
		FString(), // Api Key
		TEXT("Hero"), // Schema
		HeroDocument.ToSharedRef()
	);
	
	UpdateTask->OnCommandSucceed().AddLambda([](TSharedPtr<FJsonObject> UpdatedDocument)
	{
		UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Successfully updated document."));
	});
	
	UpdateTask->OnCommandFailed().AddLambda([](const int ExitCode, const FString& Output)
	{
		UE_LOG(LogFGameDataExtensionCommands, Warning, TEXT("Command run failed with exit code %d. Output %s."), ExitCode, *Output);
	});
	
	UpdateTask->Start(/* EventDispatchThread */ ENamedThreads::GameThread);

	//
	// Make sure that Task will outlive this method call.
	//
	CurrentUpdateTask = UpdateTask;
}