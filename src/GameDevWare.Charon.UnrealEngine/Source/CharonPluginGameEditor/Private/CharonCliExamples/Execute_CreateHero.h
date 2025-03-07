#pragma once
#include "GameData/UGameDataBase.h"
#include "GameData/CommandLine/FCharonCli.h"
#include "GameDataExtensionCommandsLog.h"

static TSharedPtr<ICharonTask> CurrentCreateTask;

/**
* Example creating single document in Game Data file.
*/
static void Execute_CreateHero(const TArray<UObject*> ContextSensitiveObjects)
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
	UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Create hero document at '%s'..."), *GameDataPath);
	
	const TSharedPtr<FJsonObject> HeroDocument = MakeShared<FJsonObject>();
	
	HeroDocument->SetStringField(TEXT("Id"), TEXT("SuperBoy"));
	HeroDocument->SetStringField(TEXT("Name"), TEXT("Super Boy"));
	HeroDocument->SetBoolField(TEXT("Religious"), false);
	
	// many fields omitted here

	// list of references example
	TArray<TSharedPtr<FJsonValue>> DislikeHeroes = TArray<TSharedPtr<FJsonValue>>();
	TSharedPtr<FJsonObject> CrossbowerReference = MakeShared<FJsonObject>();
	CrossbowerReference->SetStringField(TEXT("Id"), TEXT("Crossbower"));
	DislikeHeroes.Add(MakeShared<FJsonValueObject>(CrossbowerReference));
	
	HeroDocument->SetArrayField(TEXT("DislikeHeroes"), DislikeHeroes);
	
	// list of embedded documents example
	TArray<TSharedPtr<FJsonValue>> Armors = TArray<TSharedPtr<FJsonValue>>();
	
	TSharedPtr<FJsonObject> Armor1 = MakeShared<FJsonObject>();
	Armor1->SetStringField(TEXT("Id"), TEXT("SuperBoyArmor1"));
	Armor1->SetStringField(TEXT("Name"), TEXT("Textile Armor"));
	Armor1->SetNumberField(TEXT("Dodge"), 0);
	Armor1->SetNumberField(TEXT("HitPoints"), 0);

	TSharedPtr<FJsonObject> Armor2 = MakeShared<FJsonObject>();
	Armor2->SetStringField(TEXT("Id"), TEXT("SuperBoyArmor2"));
	Armor2->SetStringField(TEXT("Name"), TEXT("Kevlar Armor"));
	Armor2->SetNumberField(TEXT("Dodge"),  0.1);
	Armor2->SetNumberField(TEXT("HitPoints"), 4);

	Armors.Add(MakeShared<FJsonValueObject>(Armor1));
	Armors.Add(MakeShared<FJsonValueObject>(Armor2));
	HeroDocument->SetArrayField(TEXT("Armors"), Armors);

	//
	// Documentation for Import command and its parameters:
	// https://gamedevware.github.io/charon/advanced/commands/data_import.html
	//
	const auto CreateTask = FCharonCli::CreateDocument(
		GameDataPath,
		FString(), // Api Key
		TEXT("Hero"), // Schema
		HeroDocument.ToSharedRef()
	);
	
	CreateTask->OnCommandSucceed().AddLambda([](TSharedPtr<FJsonObject> CreatedDocument)
	{
		UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Successfully created document."));
	});
	
	CreateTask->OnCommandFailed().AddLambda([](const int ExitCode, const FString& Output)
	{
		UE_LOG(LogFGameDataExtensionCommands, Warning, TEXT("Command run failed with exit code %d. Output %s."), ExitCode, *Output);
	});
	
	CreateTask->Start(/* EventDispatchThread */ ENamedThreads::GameThread);

	//
	// Make sure that Task will outlive this method call.
	//
	CurrentCreateTask = CreateTask;
}