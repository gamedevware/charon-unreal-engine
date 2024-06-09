// Copyright GameDevWare, Denis Zykov 2024

#pragma once

#include "GameData/GameDataAssetActions/UNewGameDataFactory.h"

#include "UObject/Class.h"
#include "Misc/FeedbackContext.h"
#include "GameData/UGameDataBase.h"
#include "Serialization/MemoryReader.h"
#include "Misc/DateTime.h"
#include "HAL/FileManager.h"
#include "GameData/GameDataEditor/SCreateGameDataDialog.h"

DEFINE_LOG_CATEGORY(LogUNewGameDataFactory);

UNewGameDataFactory::UNewGameDataFactory(const FObjectInitializer& ObjectInitializer)
{
	SupportedClass = UGameDataBase::StaticClass();

	bCreateNew = true;
}

bool UNewGameDataFactory::DoesSupportClass(UClass* Class)
{
	return UGameDataBase::StaticClass() == Class || Class->IsChildOf(UGameDataBase::StaticClass());
}


bool UNewGameDataFactory::ConfigureProperties()
{
	return true;
}

UObject* UNewGameDataFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName,
                                               EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	check(InClass->IsChildOf(UGameDataBase::StaticClass()));

	if (InParent == nullptr)
	{
		Warn->Logf(ELogVerbosity::Error, TEXT("Unable to create a game data file. The parent package is not set in the FactoryCreateNew() parameters."));
		return nullptr;
	}

	FString PackageFilePath;
	FPackageName::TryConvertLongPackageNameToFilename(InParent->GetPackage()->GetName(), PackageFilePath);

	if (PackageFilePath.IsEmpty())
	{
		Warn->Logf(ELogVerbosity::Error, TEXT("Unable to create a game data file. Failed to determine asset path in file system."));
		return nullptr;
	}

	if (IsAutomatedImport())
	{
		Warn->Logf(ELogVerbosity::Error, TEXT("Unable to create a game data file in automated import mode because it require user interaction. Use UImportGameDataFactory instead."));
		return nullptr;
	}

	PackageFilePath = FPaths::ConvertRelativePathToFull(PackageFilePath);
	
	const auto CreateGameDataDialog = SNew(SCreateGameDataDialog)
		.AssetPath(PackageFilePath);
	CreateGameDataDialog->Show();

	return nullptr;
}