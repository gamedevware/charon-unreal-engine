#include "FLazyGameDataImporter.h"

#include "JsonObjectConverter.h"
#include "UImportGameDataFactory.h"

void FLazyGameDataImporter::RegisterDeferredImport(const FString& Filename, const FName& ClassName)
{
	FDeferredGameDataImport ImportRecord;
	ImportRecord.FileName = Filename;
	ImportRecord.ClassName = ClassName.ToString();
	
	WriteJsonToImportFile(ImportRecord);
}

void FLazyGameDataImporter::ContinueDeferredImport()
{
	TArray<FDeferredGameDataImport> ImportRecords;
	ReadJsonFromImportFiles(ImportRecords);

	auto ImportFactory = NewObject<UFactory>(GetTransientPackage(), UImportGameDataFactory::StaticClass());
	for (auto ImportRecord : ImportRecords)
	{
		
	}
}


const FString CharonIntermediateImportsDirectory = FPaths::ConvertRelativePathToFull(FPaths::ProjectIntermediateDir() / "Charon" / "Imports");

void WriteJsonToImportFile(const FDeferredGameDataImport& ImportRecord)
{
	IFileManager& FileManager = IFileManager::Get();
	const FString TempFilePath =  FPaths::CreateTempFilename(*CharonIntermediateImportsDirectory, TEXT(""), TEXT(".json"));
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

void ReadJsonFromImportFiles(TArray<FDeferredGameDataImport>& OutImportRecords)
{
	IFileManager& FileManager = IFileManager::Get();

	TArray<FString> FoundFiles;
	FileManager.FindFiles(FoundFiles, *CharonIntermediateImportsDirectory, TEXT(".json"));

	for(auto FileName : FoundFiles)
	{
		const TUniquePtr<FArchive> JsonFileStream = TUniquePtr<FArchive>(FileManager.CreateFileReader(*FileName, EFileRead::FILEREAD_NoFail));
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
		FDeferredGameDataImport OutObject;
		if (!FJsonObjectConverter::JsonObjectToUStruct<FDeferredGameDataImport>(OutJsonObject->ToSharedRef(), &OutObject, 0, 0, false, &FailReason))
		{
			continue;
		}
		OutImportRecords.Add(OutObject);
	}
}