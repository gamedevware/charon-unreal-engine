﻿// Copyright GameDevWare, Denis Zykov 2025

#include "GameData/CommandLine/FCharonCli.h"
#include "GameData/CommandLine/TPreparedCliCommand.h"

#include "GameData/CommandLine/FCharonCliCommandRunner.h"
#include "GameData/CommandLine/FT4CommandRunner.h"
#include "Serialization/JsonSerializer.h"

DEFINE_LOG_CATEGORY(LogFCharonCli);

TSharedRef<TPreparedCliCommand<TSharedPtr<FJsonObject>>> FCharonCli::CreateDocument(
	const FString& GameDataUrl,
	const FString& ApiKey,
	const FString& SchemaNameOrId,
	const TSharedRef<FJsonObject>& Document,
	const ECharonLogLevel LogsVerbosity)
{
	const FString TempOutputFile = PrepareTempOutputFile();
	const FString TempInputFile = WriteJsonToTempFile(Document);
	const FString Params = FString::Format(
		TEXT("DATA CREATE --dataBase \"{0}\" --input \"{1}\" --inputFormat json --schema \"{2}\" --output \"{3}\" --outputFormat json {4}"),
		{
			GameDataUrl,
			TempInputFile,
			SchemaNameOrId,
			TempOutputFile,
			GetLogOptions(LogsVerbosity)
		});

	const TSharedRef<FCharonCliCommandRunner> CommandRunner = MakeShared<FCharonCliCommandRunner>(Params);
	CommandRunner->SetApiKey(ApiKey);
	CommandRunner->AttachTemporaryFile(TempInputFile);
	CommandRunner->AttachTemporaryFile(TempOutputFile);
	return MakeShared<TPreparedCliCommand<TSharedPtr<FJsonObject>>>(CommandRunner, INVTEXT("Creating document"),
	                                                                TempOutputFile);
}

TSharedRef<TPreparedCliCommand<TSharedPtr<FJsonObject>>> FCharonCli::UpdateDocument(
	const FString& GameDataUrl,
	const FString& ApiKey,
	const FString& SchemaNameOrId,
	const TSharedRef<FJsonObject>& Document,
	const FString& Id,
	const ECharonLogLevel LogsVerbosity)
{
	const FString TempOutputFile = PrepareTempOutputFile();
	const FString TempInputFile = WriteJsonToTempFile(Document);
	const FString Params = FString::Format(
		TEXT("DATA UPDATE --dataBase \"{0}\" --input \"{1}\" --inputFormat json --schema \"{2}\" --id \"{3}\" --output \"{4}\" --outputFormat json {5}"),
		{
			GameDataUrl,
			TempInputFile,
			SchemaNameOrId,
			Id,
			TempOutputFile,
			GetLogOptions(LogsVerbosity)
		});

	const TSharedRef<FCharonCliCommandRunner> CommandRunner = MakeShared<FCharonCliCommandRunner>(Params);
	CommandRunner->SetApiKey(ApiKey);
	CommandRunner->AttachTemporaryFile(TempInputFile);
	CommandRunner->AttachTemporaryFile(TempOutputFile);
	return MakeShared<TPreparedCliCommand<TSharedPtr<FJsonObject>>>(CommandRunner, INVTEXT("Updating document"),
	                                                                TempOutputFile);
}

TSharedRef<TPreparedCliCommand<TSharedPtr<FJsonObject>>> FCharonCli::DeleteDocument(
	const FString& GameDataUrl,
	const FString& ApiKey,
	const FString& SchemaNameOrId,
	const TSharedRef<FJsonObject>& Document,
	const ECharonLogLevel LogsVerbosity)
{
	const auto IdValue = Document->TryGetField(TEXT("Id"));
	FString Id;
	switch (IdValue->Type)
	{
	case EJson::String:
		IdValue->TryGetString(Id);
		break;
	case EJson::Number:
		double IdNumber; 
		IdValue->TryGetNumber(IdNumber);
		Id = FString::Format(TEXT("{0}"), { IdNumber });
		break;
	case EJson::Boolean:
		bool IdBoolean; 
		IdValue->TryGetBool(IdBoolean);
		Id = FString::Format(TEXT("{0}"), { IdBoolean });
		break;
	default:
	case EJson::None: 
	case EJson::Null:
	case EJson::Array:
	case EJson::Object:
		Id = TEXT("null");
		break;
	}
	
	const FString TempOutputFile = PrepareTempOutputFile();
	const FString Params = FString::Format(
		TEXT("DATA DELETE --dataBase \"{0}\" --schema \"{1}\" --id \"{2}\" --output \"{3}\" --outputFormat json {4}"), {
			GameDataUrl,
			SchemaNameOrId,
			Id,
			TempOutputFile,
			GetLogOptions(LogsVerbosity)
		});

	const TSharedRef<FCharonCliCommandRunner> CommandRunner = MakeShared<FCharonCliCommandRunner>(Params);
	CommandRunner->SetApiKey(ApiKey);
	CommandRunner->AttachTemporaryFile(TempOutputFile);
	return MakeShared<TPreparedCliCommand<TSharedPtr<FJsonObject>>>(CommandRunner, INVTEXT("Deleting document"),
	                                                                TempOutputFile);
}

TSharedRef<TPreparedCliCommand<TSharedPtr<FJsonObject>>> FCharonCli::DeleteDocument(
	const FString& GameDataUrl,
	const FString& ApiKey,
	const FString& SchemaNameOrId,
	const FString& Id,
	const ECharonLogLevel LogsVerbosity)
{
	const FString TempOutputFile = PrepareTempOutputFile();
	const FString Params = FString::Format(
		TEXT("DATA DELETE --dataBase \"{0}\" --schema \"{1}\" --id \"{2}\" --output \"{3}\" --outputFormat json {4}"), {
			GameDataUrl,
			SchemaNameOrId,
			Id,
			TempOutputFile,
			GetLogOptions(LogsVerbosity)
		});

	const TSharedRef<FCharonCliCommandRunner> CommandRunner = MakeShared<FCharonCliCommandRunner>(Params);
	CommandRunner->SetApiKey(ApiKey);
	CommandRunner->AttachTemporaryFile(TempOutputFile);
	return MakeShared<TPreparedCliCommand<TSharedPtr<FJsonObject>>>(CommandRunner, INVTEXT("Deleting document"),
	                                                                TempOutputFile);
}

TSharedRef<TPreparedCliCommand<TSharedPtr<FJsonObject>>> FCharonCli::FindDocument(
	const FString& GameDataUrl,
	const FString& ApiKey,
	const FString& SchemaNameOrId,
	const FString& Id,
	const ECharonLogLevel LogsVerbosity)
{
	const FString TempOutputFile = PrepareTempOutputFile();
	const FString Params = FString::Format(
		TEXT("DATA FIND --dataBase \"{0}\" --schema \"{1}\" --id \"{2}\" --output \"{3}\" --outputFormat json {4}"), {
			GameDataUrl,
			SchemaNameOrId,
			Id,
			TempOutputFile,
			GetLogOptions(LogsVerbosity)
		});

	const TSharedRef<FCharonCliCommandRunner> CommandRunner = MakeShared<FCharonCliCommandRunner>(Params);
	CommandRunner->SetApiKey(ApiKey);
	CommandRunner->AttachTemporaryFile(TempOutputFile);
	return MakeShared<TPreparedCliCommand<TSharedPtr<FJsonObject>>>(CommandRunner, INVTEXT("Finding document"),
	                                                                TempOutputFile);
}

TSharedRef<TPreparedCliCommand<TSharedPtr<FJsonObject>>> FCharonCli::ListDocuments(
	const FString& GameDataUrl,
	const FString& ApiKey,
	const FString& SchemaNameOrId,
	const TArray<FDocumentFilter>& Filters,
	const TArray<FDocumentSorter>& Sorters,
	const TOptional<FString>& Path,
	const TOptional<uint32>& Skip,
	const TOptional<uint32>& Take,
	const ECharonLogLevel LogsVerbosity
)
{
	TArray<FString> FiltersParam;
	for (auto Filter : Filters)
	{
		FiltersParam.Add(Filter.PropertyName);
		FiltersParam.Add(Filter.GetOperationName());
		FiltersParam.Add(Filter.GetValueQuoted());
	}
	TArray<FString> SortersParam;
	for (auto Sorter : Sorters)
	{
		SortersParam.Add(Sorter.PropertyName);
		SortersParam.Add(Sorter.GetDirectionName());
	}

	const FString TempOutputFile = PrepareTempOutputFile();
	const FString Params = FString::Format(
		TEXT("DATA LIST --dataBase \"{0}\" --schema {1} {2} {3} {4} {5} {6} --output \"{7}\" --outputFormat json {8}"),
		{
			GameDataUrl,
			SchemaNameOrId,
			FiltersParam.Num() > 0 ? TEXT("--filters ") + FString::Join(FiltersParam, TEXT(" ")) : TEXT(""),
			SortersParam.Num() > 0 ? TEXT("--sorters ") + FString::Join(SortersParam, TEXT(" ")) : TEXT(""),
			Path.IsSet() ? TEXT("--path ") + Path.GetValue() : TEXT(""),
			Skip.IsSet() ? FString::Format(TEXT("--skip {0}"), {Skip.GetValue()}) : TEXT(""),
			Take.IsSet() ? FString::Format(TEXT("--take {0}"), {Take.GetValue()}) : TEXT(""),
			TempOutputFile,
			GetLogOptions(LogsVerbosity)
		});

	const TSharedRef<FCharonCliCommandRunner> CommandRunner = MakeShared<FCharonCliCommandRunner>(Params);
	CommandRunner->SetApiKey(ApiKey);
	CommandRunner->AttachTemporaryFile(TempOutputFile);
	return MakeShared<TPreparedCliCommand<TSharedPtr<FJsonObject>>>(CommandRunner, INVTEXT("Listing documents"),
	                                                                TempOutputFile);
}

TSharedRef<TPreparedCliCommand<FImportReport>> FCharonCli::Import(
	const FString& GameDataUrl,
	const FString& ApiKey,
	const TArray<FString>& SchemaNamesOrIds,
	const TSharedRef<FJsonObject>& DocumentsBySchemaNameOrId,
	const EImportMode ImportMode,
	const ECharonLogLevel LogsVerbosity)
{
	const FString TempOutputFile = PrepareTempOutputFile();
	FString ImportModeName;
	switch (ImportMode)
	{
	default:
	case EImportMode::CreateAndUpdate:
		ImportModeName = TEXT("CreateAndUpdate");
		break;
	case EImportMode::Create:
		ImportModeName = TEXT("Create");
		break;
	case EImportMode::Update:
		ImportModeName = TEXT("Update");
		break;
	case EImportMode::SafeUpdate:
		ImportModeName = TEXT("SafeUpdate");
		break;
	case EImportMode::Replace:
		ImportModeName = TEXT("Replace");
		break;
	case EImportMode::Delete:
		ImportModeName = TEXT("Delete");
		break;
	}
	const FString TempInputFile = WriteJsonToTempFile(DocumentsBySchemaNameOrId);
	const FString Params = FString::Format(
		TEXT("DATA IMPORT --dataBase \"{0}\" --input \"{1}\" --inputFormat json --schemas {2} --mode {3} --output \"{4}\" --outputFormat json {5}"), {
			GameDataUrl,
			TempInputFile,
			SchemaNamesOrIds.IsEmpty() ? "*" : FString::Join(SchemaNamesOrIds, TEXT(" ")),
			ImportModeName,
			TempOutputFile,
			GetLogOptions(LogsVerbosity)
		});

	const TSharedRef<FCharonCliCommandRunner> CommandRunner = MakeShared<FCharonCliCommandRunner>(Params);
	CommandRunner->SetApiKey(ApiKey);
	CommandRunner->AttachTemporaryFile(TempInputFile);
	CommandRunner->AttachTemporaryFile(TempOutputFile);
	return MakeShared<TPreparedCliCommand<FImportReport>>(CommandRunner, INVTEXT("Importing documents"), TempOutputFile);
}

TSharedRef<TPreparedCliCommand<FImportReport>> FCharonCli::ImportFromFile(
	const FString& GameDataUrl,
	const FString& ApiKey,
	const TArray<FString>& SchemaNamesOrIds,
	const EImportMode ImportMode,
	const FString& DocumentsBySchemaNameOrIdFilePath,
	const FString& Format,
	const ECharonLogLevel LogsVerbosity)
{
	const FString TempOutputFile = PrepareTempOutputFile();
	FString ImportModeName;
	switch (ImportMode)
	{
	default:
	case EImportMode::CreateAndUpdate:
		ImportModeName = TEXT("CreateAndUpdate");
		break;
	case EImportMode::Create:
		ImportModeName = TEXT("Create");
		break;
	case EImportMode::Update:
		ImportModeName = TEXT("Update");
		break;
	case EImportMode::SafeUpdate:
		ImportModeName = TEXT("SafeUpdate");
		break;
	case EImportMode::Replace:
		ImportModeName = TEXT("Replace");
		break;
	case EImportMode::Delete:
		ImportModeName = TEXT("Delete");
		break;
	}
	const FString Params = FString::Format(
		TEXT("DATA IMPORT --dataBase \"{0}\" --input \"{1}\" --inputFormat {2} --schemas {3} --mode {4} --output \"{5}\" --outputFormat json {6}"), {
			GameDataUrl,
			DocumentsBySchemaNameOrIdFilePath,
			Format,
			SchemaNamesOrIds.IsEmpty() ? "*" : FString::Join(SchemaNamesOrIds, TEXT(" ")),
			ImportModeName,
			TempOutputFile,
			GetLogOptions(LogsVerbosity)
		});

	const TSharedRef<FCharonCliCommandRunner> CommandRunner = MakeShared<FCharonCliCommandRunner>(Params);
	CommandRunner->SetApiKey(ApiKey);
	CommandRunner->AttachTemporaryFile(TempOutputFile);
	return MakeShared<TPreparedCliCommand<FImportReport>>(CommandRunner, INVTEXT("Importing documents"), TempOutputFile);
}

TSharedRef<TPreparedCliCommand<TSharedPtr<FJsonObject>>> FCharonCli::Export(
	const FString& GameDataUrl,
	const FString& ApiKey,
	const TArray<FString>& SchemaNamesOrIds,
	const TArray<FString>& Properties,
	const TArray<FString>& Languages,
	const EExportMode ExportMode,
	const ECharonLogLevel LogsVerbosity)
{
	FString ExportModeName;
	switch (ExportMode)
	{
	default:
	case EExportMode::Normal:
		ExportModeName = TEXT("Normal");
		break;
	case EExportMode::Publication:
		ExportModeName = TEXT("Publication");
		break;
	case EExportMode::Localization:
		ExportModeName = TEXT("Localization");
		break;
	case EExportMode::Extraction:
		ExportModeName = TEXT("Normal");
		break;
	}

	const FString TempOutputFile = PrepareTempOutputFile();
	const FString Params = FString::Format(
		TEXT(
			"DATA EXPORT --dataBase \"{0}\" --output \"{5}\" --outputFormat json --schemas {1} --properties {2} --languages {3} --mode {4} {6}"),
		{
			GameDataUrl,
			SchemaNamesOrIds.IsEmpty() ? "*" : FString::Join(SchemaNamesOrIds, TEXT(" ")),
			Properties.IsEmpty() ? "*" : FString::Join(Properties, TEXT(" ")),
			Languages.IsEmpty() ? "*" : FString::Join(Languages, TEXT(" ")),
			ExportModeName,
			TempOutputFile,
			GetLogOptions(LogsVerbosity)
		});

	const TSharedRef<FCharonCliCommandRunner> CommandRunner = MakeShared<FCharonCliCommandRunner>(Params);
	CommandRunner->SetApiKey(ApiKey);
	CommandRunner->AttachTemporaryFile(TempOutputFile);
	return MakeShared<TPreparedCliCommand<TSharedPtr<FJsonObject>>>(CommandRunner, INVTEXT("Exporting documents"),
	                                                                TempOutputFile);
}

TSharedRef<TPreparedCliCommand<>> FCharonCli::ExportToFile(
	const FString& GameDataUrl,
	const FString& ApiKey,
	const TArray<FString>& SchemaNamesOrIds,
	const TArray<FString>& Properties,
	const TArray<FString>& Languages,
	const EExportMode ExportMode,
	const FString& ExportedDocumentsFilePath,
	const FString& Format,
	const ECharonLogLevel LogsVerbosity)
{
	FString ExportModeName;
	switch (ExportMode)
	{
	default:
	case EExportMode::Normal:
		ExportModeName = TEXT("Normal");
		break;
	case EExportMode::Publication:
		ExportModeName = TEXT("Publication");
		break;
	case EExportMode::Localization:
		ExportModeName = TEXT("Localization");
		break;
	case EExportMode::Extraction:
		ExportModeName = TEXT("Extraction");
		break;
	}

	const FString Params = FString::Format(
		TEXT("DATA EXPORT --dataBase \"{0}\" --output \"{5}\" --outputFormat {6} --schemas {1} --properties {2} --languages {3} --mode {4} {7}"),
		{
			GameDataUrl,
			SchemaNamesOrIds.IsEmpty() ? "*" : FString::Join(SchemaNamesOrIds, TEXT(" ")),
			Properties.IsEmpty() ? "*" : FString::Join(Properties, TEXT(" ")),
			Languages.IsEmpty() ? "*" : FString::Join(Languages, TEXT(" ")),
			ExportModeName,
			ExportedDocumentsFilePath,
			Format,
			GetLogOptions(LogsVerbosity)
		});

	const TSharedRef<FCharonCliCommandRunner> CommandRunner = MakeShared<FCharonCliCommandRunner>(Params);
	CommandRunner->SetApiKey(ApiKey);
	return MakeShared<TPreparedCliCommand<>>(CommandRunner, INVTEXT("Exporting documents"));
}

TSharedRef<TPreparedCliCommand<FImportReport>> FCharonCli::I18NImport(
	const FString& GameDataUrl,
	const FString& ApiKey,
	const TArray<FString>& SchemaNamesOrIds,
	const TArray<FString>& LanguageIds,
	const TSharedRef<FJsonObject>& DocumentsBySchemaNameOrId,
	const ECharonLogLevel LogsVerbosity)
{
	const FString TempOutputFile = PrepareTempOutputFile();
	const FString TempInputFile = WriteJsonToTempFile(DocumentsBySchemaNameOrId);
	const FString Params = FString::Format(
		TEXT("DATA I18N IMPORT --dataBase \"{0}\" --schemas {1} --languages {2} --inputFormat json --input \"{3}\" --output \"{4}\" --outputFormat json {5}"),
		{
			GameDataUrl,
			SchemaNamesOrIds.IsEmpty() ? "*" : FString::Join(SchemaNamesOrIds, TEXT(" ")),
			LanguageIds.IsEmpty() ? "*" : FString::Join(LanguageIds, TEXT(" ")),
			TempInputFile,
			TempOutputFile,
			GetLogOptions(LogsVerbosity)
		});

	const TSharedRef<FCharonCliCommandRunner> CommandRunner = MakeShared<FCharonCliCommandRunner>(Params);
	CommandRunner->SetApiKey(ApiKey);
	CommandRunner->AttachTemporaryFile(TempInputFile);
	CommandRunner->AttachTemporaryFile(TempOutputFile);
	return MakeShared<TPreparedCliCommand<FImportReport>>(CommandRunner, INVTEXT("Importing documents"), TempOutputFile);
}

TSharedRef<TPreparedCliCommand<FImportReport>> FCharonCli::I18NImportFromFile(
	const FString& GameDataUrl,
	const FString& ApiKey,
	const TArray<FString>& SchemaNamesOrIds,
	const TArray<FString>& LanguageIds,
	const FString& DocumentsBySchemaNameOrIdFilePath,
	const FString& Format,
	const ECharonLogLevel LogsVerbosity)
{
	const FString TempOutputFile = PrepareTempOutputFile();
	const FString Params = FString::Format(
		TEXT("DATA I18N IMPORT --dataBase \"{0}\" --schemas {1} --languages {2} --inputFormat {3} --input \"{4}\" --output \"{5}\" --outputFormat json {6}"),
		{
			GameDataUrl,
			SchemaNamesOrIds.IsEmpty() ? "*" : FString::Join(SchemaNamesOrIds, TEXT(" ")),
			LanguageIds.IsEmpty() ? "*" : FString::Join(LanguageIds, TEXT(" ")),
			Format,
			DocumentsBySchemaNameOrIdFilePath,
			TempOutputFile,
			GetLogOptions(LogsVerbosity)
		});

	const TSharedRef<FCharonCliCommandRunner> CommandRunner = MakeShared<FCharonCliCommandRunner>(Params);
	CommandRunner->SetApiKey(ApiKey);
	CommandRunner->AttachTemporaryFile(TempOutputFile);
	return MakeShared<TPreparedCliCommand<FImportReport>>(CommandRunner, INVTEXT("Importing documents"), TempOutputFile);
}

TSharedRef<TPreparedCliCommand<TSharedPtr<FJsonObject>>> FCharonCli::I18NExport(
	const FString& GameDataUrl,
	const FString& ApiKey,
	const TArray<FString>& SchemaNamesOrIds,
	const FString& SourceLanguage,
	const FString& TargetLanguage,
	const ECharonLogLevel LogsVerbosity)
{
	const FString TempOutputFile = PrepareTempOutputFile();
	const FString Params = FString::Format(
		TEXT(
			"DATA I18N EXPORT --dataBase \"{0}\" --schemas {1} --sourceLanguage {2} --targetLanguage {3} --output  \"{4}\" --outputFormat json {5}"),
		{
			GameDataUrl,
			SchemaNamesOrIds.IsEmpty() ? "*" : FString::Join(SchemaNamesOrIds, TEXT(" ")),
			SourceLanguage,
			TargetLanguage,
			TempOutputFile,
			GetLogOptions(LogsVerbosity)
		});

	const TSharedRef<FCharonCliCommandRunner> CommandRunner = MakeShared<FCharonCliCommandRunner>(Params);
	CommandRunner->SetApiKey(ApiKey);
	CommandRunner->AttachTemporaryFile(TempOutputFile);
	return MakeShared<TPreparedCliCommand<TSharedPtr<FJsonObject>>>(CommandRunner, INVTEXT("Exporting documents"),
	                                                                TempOutputFile);
}

TSharedRef<TPreparedCliCommand<>> FCharonCli::I18NExportToFile(
	const FString& GameDataUrl,
	const FString& ApiKey,
	const TArray<FString>& SchemaNamesOrIds,
	const FString& SourceLanguage,
	const FString& TargetLanguage,
	const FString& ExportedDocumentsFilePath,
	const FString& Format,
	const ECharonLogLevel LogsVerbosity)
{
	const FString Params = FString::Format(
		TEXT("DATA I18N EXPORT --dataBase \"{0}\" --schemas {1} --sourceLanguage {2} --targetLanguage {3} --output \"{4}\" --outputFormat {5} {6}"),
		{
			GameDataUrl,
			SchemaNamesOrIds.IsEmpty() ? "*" : FString::Join(SchemaNamesOrIds, TEXT(" ")),
			SourceLanguage,
			TargetLanguage,
			ExportedDocumentsFilePath,
			Format,
			GetLogOptions(LogsVerbosity)
		});

	const TSharedRef<FCharonCliCommandRunner> CommandRunner = MakeShared<FCharonCliCommandRunner>(Params);
	CommandRunner->SetApiKey(ApiKey);
	return MakeShared<TPreparedCliCommand<>>(CommandRunner, INVTEXT("Exporting documents"));
}

TSharedRef<TPreparedCliCommand<>> FCharonCli::I18NAddLanguage(
	const FString& GameDataUrl,
	const FString& ApiKey,
	const TArray<FString>& LanguageIds,
	const ECharonLogLevel LogsVerbosity)
{
	const FString Params = FString::Format(
		TEXT("DATA I18N ADDLANGUAGE --dataBase \"{0}\" --languages {1} {2}"), {
		   GameDataUrl,
		   FString::Join(LanguageIds, TEXT(" ")),
		   GetLogOptions(LogsVerbosity)
		});

	const TSharedRef<FCharonCliCommandRunner> CommandRunner = MakeShared<FCharonCliCommandRunner>(Params);
	CommandRunner->SetApiKey(ApiKey);
	return MakeShared<TPreparedCliCommand<>>(CommandRunner, INVTEXT("Adding translation languages"));
}

TSharedRef<TPreparedCliCommand<FString>> FCharonCli::I18NListLanguages
(
	const FString& GameDataUrl,
	const FString& ApiKey,
	const ECharonLogLevel LogsVerbosity
)
{
	const FString TempOutputFile = PrepareTempOutputFile();
	const FString Params = FString::Format(
	TEXT("DATA I18N LANGUAGES --dataBase \"{0}\" --output {1} --outputFormat list {2}"), {
	   GameDataUrl,
	   TempOutputFile,
	   GetLogOptions(LogsVerbosity)
	});

	const TSharedRef<FCharonCliCommandRunner> CommandRunner = MakeShared<FCharonCliCommandRunner>(Params);
	CommandRunner->SetApiKey(ApiKey);
	CommandRunner->AttachTemporaryFile(TempOutputFile);
	
	return MakeShared<TPreparedCliCommand<FString>>(CommandRunner, INVTEXT("Listing translation languages"), TempOutputFile);
}

TSharedRef<TPreparedCliCommand<TSharedPtr<FJsonObject>>> FCharonCli::CreatePatch(
	const FString& GameDataUrl1,
	const FString& GameDataUrl2,
	const FString& ApiKey,
	const ECharonLogLevel LogsVerbosity
)
{
	const FString TempOutputFile = PrepareTempOutputFile();
	const FString Params = FString::Format(
		TEXT("DATA CREATEPATCH --dataBase1 \"{0}\" --dataBase2 \"{1}\" --output  \"{2}\" --outputFormat json {3}"), {
			GameDataUrl1,
			GameDataUrl2,
			TempOutputFile,
			GetLogOptions(LogsVerbosity)
		});

	const TSharedRef<FCharonCliCommandRunner> CommandRunner = MakeShared<FCharonCliCommandRunner>(Params);
	CommandRunner->SetApiKey(ApiKey);
	CommandRunner->AttachTemporaryFile(TempOutputFile);
	return MakeShared<TPreparedCliCommand<TSharedPtr<FJsonObject>>>(CommandRunner, INVTEXT("Creating a patch"),
	                                                                TempOutputFile);
}

TSharedRef<TPreparedCliCommand<>> FCharonCli::CreatePatchToFile(
	const FString& GameDataUrl1,
	const FString& GameDataUrl2,
	const FString& ApiKey,
	const FString& PathDocumentFilePath,
	const FString& Format,
	const ECharonLogLevel LogsVerbosity)
{
	const FString Params = FString::Format(
		TEXT("DATA CREATEPATCH --dataBase1 \"{0}\" --dataBase2 \"{1}\"  --output \"{2}\" --outputFormat {3} {4}"), {
			GameDataUrl1,
			GameDataUrl2,
			PathDocumentFilePath,
			Format,
			GetLogOptions(LogsVerbosity)
		});

	const TSharedRef<FCharonCliCommandRunner> CommandRunner = MakeShared<FCharonCliCommandRunner>(Params);
	CommandRunner->SetApiKey(ApiKey);
	return MakeShared<TPreparedCliCommand<>>(CommandRunner, INVTEXT("Creating a patch"));
}

TSharedRef<TPreparedCliCommand<>> FCharonCli::ApplyPatch(
	const FString& GameDataUrl,
	const FString& ApiKey,
	const TSharedRef<FJsonObject>& GameDataPatch,
	const ECharonLogLevel LogsVerbosity)
{
	const FString TempInputFile = WriteJsonToTempFile(GameDataPatch);
	const FString Params = FString::Format(
		TEXT("DATA APPLYPATCH --dataBase \"{0}\" --input \"{1}\" --inputFormat json {2}"), {
			GameDataUrl,
			TempInputFile,
			GetLogOptions(LogsVerbosity)
		});

	const TSharedRef<FCharonCliCommandRunner> CommandRunner = MakeShared<FCharonCliCommandRunner>(Params);
	CommandRunner->SetApiKey(ApiKey);
	CommandRunner->AttachTemporaryFile(TempInputFile);
	return MakeShared<TPreparedCliCommand<>>(CommandRunner, INVTEXT("Applying a patch"));
}

TSharedRef<TPreparedCliCommand<>> FCharonCli::ApplyPatchFromFile(
	const FString& GameDataUrl,
	const FString& ApiKey,
	const FString& PathDocumentFilePath,
	const FString& Format,
	const ECharonLogLevel LogsVerbosity)
{
	const FString Params = FString::Format(
		TEXT("DATA APPLYPATCH --dataBase \"{0}\" --input \"{1}\" --inputFormat {2} {3}"), {
			GameDataUrl,
			PathDocumentFilePath,
			Format,
			GetLogOptions(LogsVerbosity)
		});

	const TSharedRef<FCharonCliCommandRunner> CommandRunner = MakeShared<FCharonCliCommandRunner>(Params);
	CommandRunner->SetApiKey(ApiKey);
	return MakeShared<TPreparedCliCommand<>>(CommandRunner, INVTEXT("Applying a patch"));
}

TSharedRef<TPreparedCliCommand<TSharedPtr<FJsonObject>>> FCharonCli::Backup(
	const FString& GameDataUrl,
	const FString& ApiKey,
	const ECharonLogLevel LogsVerbosity)
{
	const FString TempOutputFile = PrepareTempOutputFile();
	const FString Params = FString::Format(
		TEXT("DATA BACKUP --dataBase \"{0}\" --output {1} --outputFormat json {2}"), {
			GameDataUrl,
			TempOutputFile,
			GetLogOptions(LogsVerbosity)
		});

	const TSharedRef<FCharonCliCommandRunner> CommandRunner = MakeShared<FCharonCliCommandRunner>(Params);
	CommandRunner->SetApiKey(ApiKey);
	CommandRunner->AttachTemporaryFile(TempOutputFile);
	return MakeShared<TPreparedCliCommand<TSharedPtr<FJsonObject>>>(CommandRunner, INVTEXT("DATA BACKUP"),
	                                                                TempOutputFile);
}

TSharedRef<TPreparedCliCommand<>> FCharonCli::BackupToFile(
	const FString& GameDataUrl,
	const FString& ApiKey,
	const FString& GameDataFilePath,
	const FString& Format,
	const ECharonLogLevel LogsVerbosity)
{
	const FString Params = FString::Format(
		TEXT("DATA BACKUP --dataBase \"{0}\" --output \"{1}\" --outputFormat {2} {3}"), {
			GameDataUrl,
			GameDataFilePath,
			Format,
			GetLogOptions(LogsVerbosity)
		});

	const TSharedRef<FCharonCliCommandRunner> CommandRunner = MakeShared<FCharonCliCommandRunner>(Params);
	CommandRunner->SetApiKey(ApiKey);
	return MakeShared<TPreparedCliCommand<>>(CommandRunner, INVTEXT("Making backup"));
}

TSharedRef<TPreparedCliCommand<>> FCharonCli::Restore(
	const FString& GameDataUrl,
	const FString& ApiKey,
	const TSharedRef<FJsonObject>& GameData,
	const ECharonLogLevel LogsVerbosity)
{
	const FString TempInputFile = WriteJsonToTempFile(GameData);
	const FString Params = FString::Format(
		TEXT("DATA RESTORE --dataBase \"{0}\" --input \"{1}\" --inputFormat json {2}"), {
			GameDataUrl,
			TempInputFile,
			GetLogOptions(LogsVerbosity)
		});

	const TSharedRef<FCharonCliCommandRunner> CommandRunner = MakeShared<FCharonCliCommandRunner>(Params);
	CommandRunner->SetApiKey(ApiKey);
	CommandRunner->AttachTemporaryFile(TempInputFile);
	return MakeShared<TPreparedCliCommand<>>(CommandRunner, INVTEXT("Restoring backup"));
}

TSharedRef<TPreparedCliCommand<>> FCharonCli::RestoreFromFile(
	const FString& GameDataUrl,
	const FString& ApiKey,
	const FString& GameDataFilePath,
	const FString& Format,
	const ECharonLogLevel LogsVerbosity)
{
	const FString Params = FString::Format(
		TEXT("DATA RESTORE --dataBase \"{0}\" --input \"{1}\" --inputFormat {2} {3}"), {
			GameDataUrl,
			GameDataFilePath,
			Format,
			GetLogOptions(LogsVerbosity)
		});

	const TSharedRef<FCharonCliCommandRunner> CommandRunner = MakeShared<FCharonCliCommandRunner>(Params);
	CommandRunner->SetApiKey(ApiKey);
	return MakeShared<TPreparedCliCommand<>>(CommandRunner, INVTEXT("Restoring backup"));
}

TSharedRef<TPreparedCliCommand<FValidationReport>> FCharonCli::Validate(
	const FString& GameDataUrl,
	const FString& ApiKey,
	const TArray<EValidationOption>& ValidationOptions,
	const ECharonLogLevel LogsVerbosity)
{
	TArray<FString> ValidationOptionNames;
	for (const auto Option : ValidationOptions)
	{
		switch (Option)
		{
		default:
		case EValidationOption::Repair: ValidationOptionNames.Add(TEXT("Repair"));
			break;
		case EValidationOption::CheckTranslation: ValidationOptionNames.Add(TEXT("CheckTranslation"));
			break;
		case EValidationOption::DeduplicateIds: ValidationOptionNames.Add(TEXT("DeduplicateIds"));
			break;
		case EValidationOption::RepairRequiredWithDefaultValue: ValidationOptionNames.Add(
				TEXT("RepairRequiredWithDefaultValue"));
			break;
		case EValidationOption::EraseInvalidValue: ValidationOptionNames.Add(TEXT("EraseInvalidValue"));
			break;
		case EValidationOption::CheckRequirements: ValidationOptionNames.Add(TEXT("CheckRequirements"));
			break;
		case EValidationOption::CheckFormat: ValidationOptionNames.Add(TEXT("CheckFormat"));
			break;
		case EValidationOption::CheckUniqueness: ValidationOptionNames.Add(TEXT("CheckUniqueness"));
			break;
		case EValidationOption::CheckReferences: ValidationOptionNames.Add(TEXT("CheckReferences"));
			break;
		case EValidationOption::CheckSpecification: ValidationOptionNames.Add(TEXT("CheckSpecification"));
			break;
		case EValidationOption::CheckConstraints: ValidationOptionNames.Add(TEXT("CheckConstraints"));
			break;
		}
	}

	const FString TempOutputFile = PrepareTempOutputFile();
	const FString Params = FString::Format(
		TEXT("DATA VALIDATE --dataBase \"{0}\" --validationOptions {1} --output {2} --outputFormat json {3}"), {
			GameDataUrl,
			ValidationOptionNames.IsEmpty() ? "None" : FString::Join(ValidationOptionNames, TEXT(" ")),
			TempOutputFile,
			GetLogOptions(LogsVerbosity)
		});

	const TSharedRef<FCharonCliCommandRunner> CommandRunner = MakeShared<FCharonCliCommandRunner>(Params);
	CommandRunner->SetApiKey(ApiKey);
	CommandRunner->AttachTemporaryFile(TempOutputFile);
	return MakeShared<TPreparedCliCommand<FValidationReport>>(CommandRunner, INVTEXT("Validating documents"),
	                                                          TempOutputFile);
}


TSharedRef<TPreparedCliCommand<>> FCharonCli::GenerateUnrealEngineSourceCode(
	const FString& GameDataUrl,
	const FString& ApiKey,
	const FString& OutputDirectory,
	const FString& DocumentClassName,
	const FString& GameDataClassName,
	const FString& DefineConstants,
	const TArray<ESourceCodeGenerationOptimizations>& SourceCodeGenerationOptimizations,
	const ESourceCodeIndentation SourceCodeIndentation,
	const ESourceCodeLineEndings SourceCodeLineEndings,
	const bool ClearOutputDirectory,
	const ECharonLogLevel LogsVerbosity)
{
	FString SourceCodeIndentationName;
	switch (SourceCodeIndentation)
	{
	default:
	case ESourceCodeIndentation::Tabs:
		SourceCodeIndentationName = TEXT("Tabs");
		break;
	case ESourceCodeIndentation::TwoSpaces:
		SourceCodeIndentationName = TEXT("TwoSpaces");
		break;
	case ESourceCodeIndentation::FourSpaces:
		SourceCodeIndentationName = TEXT("FourSpaces");
		break;
	}

	FString SourceCodeLineEndingsName;
	switch (SourceCodeLineEndings)
	{
	default:
	case ESourceCodeLineEndings::Windows:
		SourceCodeLineEndingsName = TEXT("Windows");
		break;
	case ESourceCodeLineEndings::Unix:
		SourceCodeLineEndingsName = TEXT("Unix");
		break;
	case ESourceCodeLineEndings::OsDefault: 
#if PLATFORM_WINDOWS
		SourceCodeLineEndingsName = TEXT("Windows");
#else
		SourceCodeLineEndingsName = TEXT("Unix");
#endif
		break;
	}

	TArray<FString> SourceCodeGenerationOptimizationNames;
	for (const auto Optimization : SourceCodeGenerationOptimizations)
	{
		switch (Optimization)
		{
		default: break;
		case ESourceCodeGenerationOptimizations::EagerReferenceResolution: SourceCodeGenerationOptimizationNames.Add(
				TEXT("EagerReferenceResolution"));
			break;
		case ESourceCodeGenerationOptimizations::RawReferences: SourceCodeGenerationOptimizationNames.Add(
				TEXT("RawReferences"));
			break;
		case ESourceCodeGenerationOptimizations::RawLocalizedStrings: SourceCodeGenerationOptimizationNames.Add(
				TEXT("RawLocalizedStrings"));
			break;
		case ESourceCodeGenerationOptimizations::DisableStringPooling: SourceCodeGenerationOptimizationNames.Add(
				TEXT("DisableStringPooling"));
			break;
		case ESourceCodeGenerationOptimizations::DisableJsonSerialization: SourceCodeGenerationOptimizationNames.Add(
				TEXT("DisableJsonSerialization"));
			break;
		case ESourceCodeGenerationOptimizations::DisableMessagePackSerialization: SourceCodeGenerationOptimizationNames.
				Add(TEXT("DisableMessagePackSerialization"));
			break;
		case ESourceCodeGenerationOptimizations::DisablePatching: SourceCodeGenerationOptimizationNames.Add(
				TEXT("DisablePatching"));
			break;
		case ESourceCodeGenerationOptimizations::DisableDocumentIdEnums: SourceCodeGenerationOptimizationNames.Add(
				TEXT("DisableDocumentIdEnums"));
			break;
		}
	}

	const FString Params = FString::Format(
		TEXT("GENERATE UECPPCODE --dataBase \"{0}\" --outputDirectory \"{1}\" --documentClassName \"{2}\" --gameDataClassName \"{3}\" --defineConstants \"{4}\" --indentation \"{5}\" --lineEndings \"{6}\" --splitFiles {7} --optimizations {8} {9}"),
		{
			GameDataUrl,
			OutputDirectory,
			DocumentClassName,
			GameDataClassName,
			DefineConstants,
			SourceCodeIndentationName,
			SourceCodeLineEndingsName,
			ClearOutputDirectory ? TEXT("--clearOutputDirectory") : TEXT(""),
			SourceCodeGenerationOptimizationNames.IsEmpty()
				? ""
				: FString::Join(SourceCodeGenerationOptimizationNames, TEXT(" ")),
			GetLogOptions(LogsVerbosity)
		});

	const TSharedRef<FCharonCliCommandRunner> CommandRunner = MakeShared<FCharonCliCommandRunner>(Params);
	CommandRunner->SetApiKey(ApiKey);
	return MakeShared<TPreparedCliCommand<>>(CommandRunner, INVTEXT("Generating C++ code"));
}

TSharedRef<TPreparedCliCommand<>> FCharonCli::DumpTemplates(
	const FString& OutputDirectory
)
{
	const FString Params = FString::Format(TEXT("GENERATE TEMPLATES --outputDirectory \"{0}\""), {
		                                       OutputDirectory
	                                       });

	const TSharedRef<FCharonCliCommandRunner> CommandRunner = MakeShared<FCharonCliCommandRunner>(Params);
	return MakeShared<TPreparedCliCommand<>>(CommandRunner, INVTEXT("GENERATE TEMPLATES"));
}

TSharedRef<TPreparedCliCommand<FString>> FCharonCli::GetVersion()
{
	const FString Params = TEXT("VERSION");

	const TSharedRef<FCharonCliCommandRunner> CommandRunner = MakeShared<FCharonCliCommandRunner>(Params);
	return MakeShared<TPreparedCliCommand<FString>>(CommandRunner, INVTEXT("VERSION"));
}

TSharedRef<TPreparedCliCommand<FString>> FCharonCli::GetGameDataVersion(
	const FString& GameDataUrl,
	const FString& ApiKey,
	const ECharonLogLevel LogsVerbosity
)
{
	const FString Params = FString::Format(
		TEXT("DATA VERSION --dataBase \"{0}\" {1}"), {
           GameDataUrl,
           GetLogOptions(LogsVerbosity)
       });

	const TSharedRef<FCharonCliCommandRunner> CommandRunner = MakeShared<FCharonCliCommandRunner>(Params);
	CommandRunner->SetApiKey(ApiKey);
	return MakeShared<TPreparedCliCommand<FString>>(CommandRunner, INVTEXT("Getting game data version"));
}

TSharedRef<TPreparedCliCommand<>> FCharonCli::InitGameData(
	const FString& GameDataPath,
	const ECharonLogLevel LogsVerbosity)
{
	const FString Params = FString::Format(
	TEXT("INIT \"{0}\" {1}"), {
	   GameDataPath,
	   GetLogOptions(LogsVerbosity)
   });

	const TSharedRef<FCharonCliCommandRunner> CommandRunner = MakeShared<FCharonCliCommandRunner>(Params);
	return MakeShared<TPreparedCliCommand<>>(CommandRunner, INVTEXT("Initializing game data file"));
}

TSharedRef<TPreparedCliCommand<FString>> FCharonCli::RunCharon(
	const TArray<FString>& CommandsAndOptions,
	const FString& ApiKey)
{
	const FString Params = FString::Join(CommandsAndOptions, TEXT(" "));
	const TSharedRef<FCharonCliCommandRunner> CommandRunner = MakeShared<FCharonCliCommandRunner>(Params);
	CommandRunner->SetApiKey(ApiKey);
	return MakeShared<TPreparedCliCommand<FString>>(CommandRunner, INVTEXT("Running custom command"));
}

TSharedRef<TPreparedCliCommand<FString>> FCharonCli::RunT4(
	const FString& TemplateFile,
	const TArray<FString>& ReferencedAssemblies,
	const TArray<FString>& Usings,
	const TArray<FString>& IncludeDirectories,
	const TArray<FString>& AssemblyLookupDirectories,
	const TArray<TPair<FString, FString>>& Parameters,
	const bool bUseRelativeLinePragmas,
	const bool bDebugMode,
	const bool bVerboseLogs)
{
	FString Params = FString();
	for (auto ReferencedAssembly : ReferencedAssemblies)
	{
		Params += TEXT(" \"-r=") + ReferencedAssembly + "\"";
	}
	for (auto UsingStatement : Usings)
	{
		Params += TEXT(" \"-u=") + UsingStatement + "\"";
	}
	for (auto IncludeDirectory : IncludeDirectories)
	{
		Params += TEXT(" \"-I=") + IncludeDirectory + "\"";
	}
	for (auto AssemblyLookupDirectory : AssemblyLookupDirectories)
	{
		Params += TEXT(" \"-P=") + AssemblyLookupDirectory + "\"";
	}
	for (auto ParameterTuple : Parameters)
	{
		Params += TEXT(" \"-p=") + ParameterTuple.Key + "=" + ParameterTuple.Value + "\"";
	}
	if (bUseRelativeLinePragmas)
	{
		Params += TEXT(" -l");
	}
	if (bDebugMode)
	{
		Params += TEXT(" --debug");
	}
	if (bVerboseLogs)
	{
		Params += TEXT(" --verbose");
	}

	Params += TEXT(" \"") + TemplateFile + TEXT("\"");

	const TSharedRef<FT4CommandRunner> CommandRunner = MakeShared<FT4CommandRunner>(Params);
	return MakeShared<TPreparedCliCommand<FString>>(CommandRunner, INVTEXT("Custom command"));
}

TSharedRef<TPreparedCliCommand<FString>> FCharonCli::PreprocessT4(
	const FString& TemplateFile,
	const FString& OutputFile,
	const FString& TemplateClassName,
	const TArray<FString>& Usings,
	const bool bUseRelativeLinePragmas,
	const bool bDebugMode,
	const bool bVerboseLogs)
{
	FString Params = FString();
	if (!OutputFile.IsEmpty())
	{
		Params += TEXT(" \"--out=") + OutputFile + "\"";
	}
	for (auto UsingStatement : Usings)
	{
		Params += TEXT(" \"-u=") + UsingStatement + "\"";
	}
	if (!TemplateClassName.IsEmpty())
	{
		Params += TEXT(" \"-c=") + TemplateClassName + "\"";
	}
	if (bUseRelativeLinePragmas)
	{
		Params += TEXT(" -l");
	}
	if (bDebugMode)
	{
		Params += TEXT(" --debug");
	}
	if (bVerboseLogs)
	{
		Params += TEXT(" --verbose");
	}

	Params += TEXT(" \"") + TemplateFile + TEXT("\"");

	const TSharedRef<FT4CommandRunner> CommandRunner = MakeShared<FT4CommandRunner>(Params);
	return MakeShared<TPreparedCliCommand<FString>>(CommandRunner, INVTEXT("Custom command"));
}

FString FCharonCli::GetLogOptions(const ECharonLogLevel LogsVerbosity)
{
	switch (LogsVerbosity)
	{
	default:
	case ECharonLogLevel::None: return FString();
	case ECharonLogLevel::Normal: return FString("--log out");
	case ECharonLogLevel::Verbose: return FString("--log out --verbose");
	}
}

FString FCharonCli::WriteJsonToTempFile(const TSharedRef<FJsonObject>& JsonObject)
{
	IFileManager& FileManager = IFileManager::Get();
	const FString TempFilePath = FPaths::CreateTempFilename(FPlatformProcess::UserTempDir(), TEXT(""), TEXT(".json"));
	const TUniquePtr<FArchive> TempFileStream = TUniquePtr<FArchive>(
		FileManager.CreateFileWriter(*TempFilePath, EFileWrite::FILEWRITE_None));
	const auto JsonWriter = TJsonWriterFactory<UTF8CHAR>::Create(TempFileStream.Get());
	FJsonSerializer::Serialize(JsonObject, JsonWriter, true);

	TempFileStream->Close();

	return TempFilePath;
}

FString FCharonCli::PrepareTempOutputFile(const FString& Extension)
{
	return FPaths::CreateTempFilename(FPlatformProcess::UserTempDir(), TEXT(""), *Extension);
}
