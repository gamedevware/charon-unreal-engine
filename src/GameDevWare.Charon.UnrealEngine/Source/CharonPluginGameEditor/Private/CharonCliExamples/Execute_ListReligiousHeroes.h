// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "GameData/UGameDataBase.h"
#include "GameData/CommandLine/FCharonCli.h"
#include "GameDataExtensionCommandsLog.h"

static TSharedPtr<ICharonTask> CurrentListFilteredTask;

/**
 * Example of listing documents by specific criteria in a Game Data file.
 */
static void Execute_ListReligiousHeroes(const TArray<UObject*> ContextSensitiveObjects)
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
	UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Listing hero documents with criteria at '%s'..."), *GameDataPath);

	TArray<FDocumentFilter> Filters;
	Filters.Add(FDocumentFilter(TEXT("Religious"), EDocumentFilterOperation::Equal, TEXT("true")));
	
	TArray<FDocumentSorter> Sorters;
	Sorters.Add(FDocumentSorter(TEXT("Name.en-US"), EDocumentSortDirection::Ascending));
	
	//
	// Documentation for the ListDocuments command and its parameters:
	// https://gamedevware.github.io/charon/advanced/commands/data_list.html
	//
	const auto ListTask = FCharonCli::ListDocuments
	(
		GameDataPath,
		FString(), // API Key
		TEXT("Hero"), // Schema
		Filters, // Filter
		Sorters, // Sorter
		TOptional<FString>(), // Path
		TOptional<uint32>(), // Skip
		TOptional<uint32>() // Take
	);

	ListTask->OnCommandSucceed().AddLambda([](TSharedPtr<FJsonObject> ListedHeroes)
	{
		const auto Collections = ListedHeroes->GetObjectField(TEXT("Collections"));
		const auto HeroCollection = Collections->GetArrayField(TEXT("Hero"));
		TArray<FString> FoundHeroNames;
		for (const auto HeroValue : HeroCollection)
		{
			const auto HeroDocument = HeroValue->AsObject();
			const auto HeroName = HeroDocument->GetObjectField(TEXT("Name"));
			auto HeroNameEn = HeroName->GetStringField(TEXT("en-US")); 
			FoundHeroNames.Add(HeroNameEn);
		}
			
		UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Successfully found '%s' hero documents."), *FString::Join(FoundHeroNames, TEXT(", ")));
	});
	
	ListTask->OnCommandFailed().AddLambda([](const int ExitCode, const FString& Output)
	{
		UE_LOG(LogFGameDataExtensionCommands, Warning, TEXT("Command execution failed with exit code %d. Output: %s"), ExitCode, *Output);
	});
	
	ListTask->Start(/* EventDispatchThread */ ENamedThreads::GameThread);

	//
	// Ensure the task outlives this method call by storing it in a shared pointer.
	//
	CurrentListFilteredTask = ListTask;
}