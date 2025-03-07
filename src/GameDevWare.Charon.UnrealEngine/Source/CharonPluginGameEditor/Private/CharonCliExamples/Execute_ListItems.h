// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "GameData/UGameDataBase.h"
#include "GameData/CommandLine/FCharonCli.h"
#include "GameDataExtensionCommandsLog.h"

static TSharedPtr<ICharonTask> CurrentListTask;

/**
 * Example of listing root and embedded documents in a Game Data file.
 */
static void Execute_ListItems(const TArray<UObject*> ContextSensitiveObjects)
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
	UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Listing item documents at '%s'..."), *GameDataPath);
	
	//
	// Documentation for the ListDocuments command and its parameters:
	// https://gamedevware.github.io/charon/advanced/commands/data_list.html
	//
	const auto ListTask = FCharonCli::ListDocuments
	(
		GameDataPath,
		FString(), // API Key
		TEXT("Item"), // Schema
		TArray<FDocumentFilter>(), // Filter
		TArray<FDocumentSorter>(), // Sorter
		TOptional<FString>(TEXT("*")), // Path: This option value causes the query to return all documents, including embedded ones.
		TOptional<uint32>(), // Skip
		TOptional<uint32>() // Take
	);
	
	ListTask->OnCommandSucceed().AddLambda([](TSharedPtr<FJsonObject> ListedItems)
	{
		const auto Collections = ListedItems->GetObjectField(TEXT("Collections"));
		const auto ItemCollection = Collections->GetArrayField(TEXT("Item"));
		TArray<FString> FoundItemNames;
		for (const auto ItemValue : ItemCollection)
		{
			const auto ItemDocument = ItemValue->AsObject();
			const auto ItemName = ItemDocument->GetObjectField(TEXT("Name"));
			auto ItemNameEn = ItemName->GetStringField(TEXT("en-US")); 
			FoundItemNames.Add(ItemNameEn);
		}
		
		UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Successfully found '%s' item documents."), *FString::Join(FoundItemNames, TEXT(", ")));
	});
	
	ListTask->OnCommandFailed().AddLambda([](const int ExitCode, const FString& Output)
	{
		UE_LOG(LogFGameDataExtensionCommands, Warning, TEXT("Command execution failed with exit code %d. Output: %s"), ExitCode, *Output);
	});
	
	ListTask->Start(/* EventDispatchThread */ ENamedThreads::GameThread);

	//
	// Ensure the task outlives this method call by storing it in a shared pointer.
	//
	CurrentListTask = ListTask;
}