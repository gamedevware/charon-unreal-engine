#include "FDeferredGameDataImporter.h"

#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "JsonObjectConverter.h"
#include "GameDataAssetActions/UImportGameDataFactory.h"
#include "UObject/SavePackage.h"

DEFINE_LOG_CATEGORY(LogFDeferredGameDataImporter);

void FDeferredGameDataImporter::RegisterGameDataToImport(const FString& FileName, const FString& ModuleName, const FString& ClassName)
{
	if (FileName.IsEmpty() || ModuleName.IsEmpty() || ClassName.IsEmpty())
	{
		return;
	}

	FDeferredGameDataImportRecord ImportRecord;
	ImportRecord.FileName = FileName;
	ImportRecord.ModuleName = ModuleName;
	ImportRecord.ClassName = ClassName;

	WriteJsonToImportFile(ImportRecord);
}

void FDeferredGameDataImporter::ContinueDeferredImports()
{
	TArray<FDeferredGameDataImportRecord> ImportRecords;
	ReadJsonFromImportFiles(ImportRecords);

	for (auto ImportRecord : ImportRecords)
	{
		if (TryToImportGameData(ImportRecord.FileName, ImportRecord.ModuleName, ImportRecord.ClassName))
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to import game data at '%s'. Review 'Output Log' for detauls."), *ImportRecord.FileName);
		}
	}
}

bool FDeferredGameDataImporter::TryToImportGameData(const FString& FileName, const FString& ModuleName, const FString& ClassName)
{
	if (FileName.IsEmpty() || ModuleName.IsEmpty() || ClassName.IsEmpty())
	{
		return false;
	}

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	IModuleInterface* _ = FModuleManager::Get().LoadModule(FName(ModuleName), ELoadModuleFlags::LogFailures);
	UClass* GameDataClass = FindFirstObjectSafe<UClass>(*ClassName);
	if (!GameDataClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to import game data at '%s' because class '%s' from module '%s' is not found."), *FileName, *ClassName, *ModuleName);
		return false;
	}

	auto BaseFilename = FPaths::GetBaseFilename(FileName);
	auto GameDataPath = FileName;

	auto ImportSettings = NewObject<UAutomatedAssetImportData>();
	ImportSettings->GroupName = TEXT("Game Data");

	ImportSettings->Filenames.Add(FPaths::ConvertRelativePathToFull(FPaths::ProjectDir(), GameDataPath));

	FString DestinationPath = FPaths::GetPath(GameDataPath);
	FPaths::MakePathRelativeTo(DestinationPath, TEXT("Content/"));
	if (DestinationPath.IsEmpty())
	{
		DestinationPath = "/Game";
	}
	else
	{
		DestinationPath = "/Game/" + DestinationPath;
	}
	ImportSettings->DestinationPath = DestinationPath;
	ImportSettings->bReplaceExisting = true;

	UImportGameDataFactory* ImportFactory = NewObject<UImportGameDataFactory>(GetTransientPackage(), UImportGameDataFactory::StaticClass());
	ImportFactory->GameDataClass = GameDataClass;
	ImportFactory->ConfigureProperties();
	ImportSettings->Factory = TObjectPtr<UFactory>(ImportFactory);

	auto ImportedAssets = AssetToolsModule.Get().ImportAssetsAutomated(ImportSettings);

	for (auto ImportedAsset : ImportedAssets)
	{
		UPackage* Package = ImportedAsset->GetOutermost();
		if (ImportedAsset->MarkPackageDirty() && Package)
		{
			FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());

			FSavePackageArgs SaveArgs;
			Package->Save(Package, ImportedAsset, *PackageFileName, SaveArgs);
		}
	}

	return ImportedAssets.Num() > 0;
}

const FString CharonIntermediateImportsDirectory = FPaths::ConvertRelativePathToFull(FPaths::ProjectIntermediateDir() / "Charon" / "Imports");

void FDeferredGameDataImporter::WriteJsonToImportFile(const FDeferredGameDataImportRecord& ImportRecord)
{
	IFileManager& FileManager = IFileManager::Get();
	const FString TempFilePath = FPaths::CreateTempFilename(*CharonIntermediateImportsDirectory, TEXT(""), TEXT(".json"));
	const TUniquePtr<FArchive> TempFileStream = TUniquePtr<FArchive>(FileManager.CreateFileWriter(*TempFilePath, EFileWrite::FILEWRITE_None));
	const auto JsonWriter = TJsonWriterFactory<UTF8CHAR>::Create(TempFileStream.Get());

	auto JsonObject = FJsonObjectConverter::UStructToJsonObject(ImportRecord);
	if (!JsonObject.IsValid())
	{
		return;
	}

	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter, true);
	TempFileStream->Close();
}

void FDeferredGameDataImporter::ReadJsonFromImportFiles(TArray<FDeferredGameDataImportRecord>& OutImportRecords)
{
	IFileManager& FileManager = IFileManager::Get();

	TArray<FString> FoundFiles;
	FileManager.FindFiles(FoundFiles, *CharonIntermediateImportsDirectory, TEXT(".json"));

	for (auto FileName : FoundFiles)
	{
		const FString FullFileName = FPaths::Combine(CharonIntermediateImportsDirectory, FileName);
		const TUniquePtr<FArchive> JsonFileStream = TUniquePtr<FArchive>(FileManager.CreateFileReader(*FullFileName, EFileRead::FILEREAD_NoFail));
		if (!JsonFileStream.IsValid())
		{
			continue;
		}

		const auto JsonReader = TJsonReaderFactory<UTF8CHAR>::Create(JsonFileStream.Get());

		TSharedPtr<FJsonValue> OutValue;
		if (!FJsonSerializer::Deserialize(JsonReader, OutValue) || !OutValue.IsValid())
		{
			continue;
		}

		TSharedPtr<FJsonObject>* OutJsonObject;
		if (!OutValue->TryGetObject(OutJsonObject) || !OutJsonObject->IsValid())
		{
			continue;
		}

		FText FailReason;
		FDeferredGameDataImportRecord OutObject;
		if (!FJsonObjectConverter::JsonObjectToUStruct<FDeferredGameDataImportRecord>(OutJsonObject->ToSharedRef(), &OutObject, 0, 0, false, &FailReason))
		{
			continue;
		}
		OutImportRecords.Add(OutObject);
	}

	for (auto FileName : FoundFiles)
	{
		const FString FullFileName = FPaths::Combine(CharonIntermediateImportsDirectory, FileName);

		FileManager.Delete(*FullFileName);
	}
}
