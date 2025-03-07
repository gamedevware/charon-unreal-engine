// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "GameData/UGameDataBase.h"
#include "GameData/CommandLine/FCharonCli.h"
#include "GameDataExtensionCommandsLog.h"

static TSharedPtr<ICharonTask> CurrentCreateTask;

/**
 * Example of creating a single document in a Game Data file.
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
	UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Creating hero document at '%s'..."), *GameDataPath);
	
	const TSharedPtr<FJsonObject> HeroDocument = MakeShared<FJsonObject>();
	
	HeroDocument->SetStringField(TEXT("Id"), TEXT("SuperBoy"));
	HeroDocument->SetStringField(TEXT("Name"), TEXT("Super Boy"));
	HeroDocument->SetBoolField(TEXT("Religious"), false);
	
	// Many fields omitted here

	// Example: List of references
	TArray<TSharedPtr<FJsonValue>> DislikeHeroes = TArray<TSharedPtr<FJsonValue>>();
	TSharedPtr<FJsonObject> CrossbowerReference = MakeShared<FJsonObject>();
	CrossbowerReference->SetStringField(TEXT("Id"), TEXT("Crossbower"));
	DislikeHeroes.Add(MakeShared<FJsonValueObject>(CrossbowerReference));
	
	HeroDocument->SetArrayField(TEXT("DislikeHeroes"), DislikeHeroes);
	
	// Example: List of embedded documents
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
	// Documentation for the CreateDocument command and its parameters:
	// https://gamedevware.github.io/charon/advanced/commands/data_create.html
	//
	const auto CreateTask = FCharonCli::CreateDocument(
		GameDataPath,
		FString(), // API Key
		TEXT("Hero"), // Schema
		HeroDocument.ToSharedRef()
	);
	
	CreateTask->OnCommandSucceed().AddLambda([](TSharedPtr<FJsonObject> CreatedDocument)
	{
		UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Successfully created document."));
	});
	
	CreateTask->OnCommandFailed().AddLambda([](const int ExitCode, const FString& Output)
	{
		UE_LOG(LogFGameDataExtensionCommands, Warning, TEXT("Command execution failed with exit code %d. Output: %s."), ExitCode, *Output);
	});
	
	CreateTask->Start(/* EventDispatchThread */ ENamedThreads::GameThread);

	//
	// Ensure the task outlives this method call by storing it in a shared pointer.
	//
	CurrentCreateTask = CreateTask;
}
