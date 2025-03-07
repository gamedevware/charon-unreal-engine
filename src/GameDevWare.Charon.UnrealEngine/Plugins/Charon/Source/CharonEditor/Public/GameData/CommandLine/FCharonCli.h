// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "ECharonLogLevel.h"
#include "EExportMode.h"
#include "EImportMode.h"
#include "ESourceCodeGenerationOptimizations.h"
#include "ESourceCodeIndentation.h"
#include "ESourceCodeLineEndings.h"
#include "EValidationOption.h"
#include "FDocumentFilter.h"
#include "FDocumentSorter.h"
#include "FValidationReport.h"
#include "TPreparedCliCommand.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFCharonCli, Log, All);

/**
 * @class FCharonCli
 * @brief Provides a convenient interface for running Charon.exe command line operations.
 *
 * This class encapsulates functionality for creating, updating, deleting, importing, exporting and finding documents
 * within a specified GameData URL, either file-based or server-based. It simplifies interactions
 * with the Charon command line tool, offering methods that return tasks representing the operations.
 * For more detailed documentation of each method, refer to the Charon command line documentation at
 * https://gamedevware.github.io/charon/advanced/command_line.html
 */
class CHARONEDITOR_API FCharonCli final
{
public:
	/**
	 * Creates a document in the specified GameData URL.
	 *
	 * @param GameDataUrl The URL of the GameData file or server.
	 * @param ApiKey Authentication credentials if GameDataUrl is a server, otherwise empty.
	 * @param SchemaNameOrId The schema name or ID of the document.
	 * @param Document The document to create as a shared reference to an FJsonObject.
	 * @param LogsVerbosity The verbosity level of logs. Defaults to ECharonLogLevel::Normal.
	 * @return A shared reference to a TPreparedCliCommand representing the create operation. 
	 *         The command, upon completion, provides the created document.
	 */
	static TSharedRef<TPreparedCliCommand<TSharedPtr<FJsonObject>>> CreateDocument(
		const FString& GameDataUrl,
		const FString& ApiKey,
		const FString& SchemaNameOrId,
		const TSharedRef<FJsonObject>& Document,
		const ECharonLogLevel LogsVerbosity = ECharonLogLevel::Normal
	);

	/**
	 * Updates a document in the specified GameData URL.
	 *
	 * @param GameDataUrl The URL of the GameData file or server.
	 * @param ApiKey Authentication credentials if GameDataUrl is a server, otherwise empty.
	 * @param SchemaNameOrId The schema name or ID of the document.
	 * @param Document The document to update as a shared reference to an FJsonObject.
	 * @param Id Optional ID of the document to update if not present in the Document object.
	 * @param LogsVerbosity The verbosity level of logs. Defaults to ECharonLogLevel::Normal.
	 * @return A shared reference to a TPreparedCliCommand representing the update operation. 
	 *         The command, upon completion, provides the updated document.
	 */
	static TSharedRef<TPreparedCliCommand<TSharedPtr<FJsonObject>>> UpdateDocument(
		const FString& GameDataUrl,
		const FString& ApiKey,
		const FString& SchemaNameOrId,
		const TSharedRef<FJsonObject>& Document,
		const FString& Id = FString(),
		const ECharonLogLevel LogsVerbosity = ECharonLogLevel::Normal
	);

    /**
     * Deletes a document in the specified GameData URL.
     *
     * @param GameDataUrl The URL of the GameData file or server.
     * @param ApiKey Authentication credentials if GameDataUrl is a server, otherwise empty.
     * @param SchemaNameOrId The schema name or ID of the document.
     * @param Document The document to delete, only the ID is used for deletion.
     * @param LogsVerbosity The verbosity level of logs. Defaults to ECharonLogLevel::Normal.
     * @return A shared reference to a TPreparedCliCommand representing the delete operation. 
     *         The command, upon completion, provides the deleted document.
     */
	static TSharedRef<TPreparedCliCommand<TSharedPtr<FJsonObject>>> DeleteDocument(
		const FString& GameDataUrl,
		const FString& ApiKey,
		const FString& SchemaNameOrId,
		const TSharedRef<FJsonObject>& Document,
		const ECharonLogLevel LogsVerbosity = ECharonLogLevel::Normal
	);

	/**
	 * Deletes a document in the specified GameData URL by ID.
	 *
	 * @param GameDataUrl The URL of the GameData file or server.
	 * @param ApiKey Authentication credentials if GameDataUrl is a server, otherwise empty.
	 * @param SchemaNameOrId The schema name or ID of the document.
	 * @param Id The ID of the document to delete.
	 * @param LogsVerbosity The verbosity level of logs. Defaults to ECharonLogLevel::Normal.
	 * @return A shared reference to a TPreparedCliCommand representing the delete operation. 
	 *         The command, upon completion, provides the deleted document.
	 */
	static TSharedRef<TPreparedCliCommand<TSharedPtr<FJsonObject>>> DeleteDocument(
		const FString& GameDataUrl,
		const FString& ApiKey,
		const FString& SchemaNameOrId,
		const FString& Id,
		const ECharonLogLevel LogsVerbosity = ECharonLogLevel::Normal
	);

	/**
	 * Finds a document in the specified GameData URL by ID.
	 *
	 * @param GameDataUrl The URL of the GameData file or server.
	 * @param ApiKey Authentication credentials if GameDataUrl is a server, otherwise empty.
	 * @param SchemaNameOrId The schema name or ID of the document.
	 * @param Id The ID of the document to find.
	 * @param LogsVerbosity The verbosity level of logs. Defaults to ECharonLogLevel::Normal.
	 * @return A shared reference to a TPreparedCliCommand representing the find operation. 
	 *         The command, upon completion, provides the found document.
	 */
	static TSharedRef<TPreparedCliCommand<TSharedPtr<FJsonObject>>> FindDocument(
		const FString& GameDataUrl,
		const FString& ApiKey,
		const FString& SchemaNameOrId,
		const FString& Id,
		const ECharonLogLevel LogsVerbosity = ECharonLogLevel::Normal
	);

	/**
	 * List a documents in the specified GameData URL.
	 *
	 * @param GameDataUrl The URL of the GameData file or server.
	 * @param ApiKey Authentication credentials if GameDataUrl is a server, otherwise empty.
	 * @param SchemaNameOrId The schema name or ID of the document.
	 * @param Filters Filters for documents to list.
	 * @param Sorters Sorters for documents to list.
	 * @param Path Limit search only to embedded documents with specified path.
	 * @param Skip Number of documents to skip before writing to output.
	 * @param Take Number of documents to take after 'skip' for output.
	 * @param LogsVerbosity The verbosity level of logs. Defaults to ECharonLogLevel::Normal.
	 * @return A shared reference to a TPreparedCliCommand representing the find operation. 
	 *         The command, upon completion, provides the found document.
	 */
	static TSharedRef<TPreparedCliCommand<TSharedPtr<FJsonObject>>> ListDocuments(
		const FString& GameDataUrl,
		const FString& ApiKey,
		const FString& SchemaNameOrId,
		const TArray<FDocumentFilter>& Filters,
		const TArray<FDocumentSorter>& Sorters,
		const TOptional<FString>& Path,
		const TOptional<uint32>& Skip,
		const TOptional<uint32>& Take,
		const ECharonLogLevel LogsVerbosity = ECharonLogLevel::Normal
	);

	/**
	 * @brief Import documents grouped by schema into a specified GameDataUrl file or server.
	 *
	 * @param GameDataUrl The URL of the GameData file or server.
	 * @param ApiKey Authentication credentials if GameDataUrl is a server, otherwise empty.
	 * @param SchemaNamesOrIds Names or IDs of schemas to import from DocumentsBySchemaNameOrId. Can be empty or '*' to import all documents.
	 * @param DocumentsBySchemaNameOrId The documents to be imported, grouped by schema name or ID.
	 * @param ImportMode The mode of import operation, see EImportMode for details.
	 * @param LogsVerbosity The verbosity level of logs. Defaults to ECharonLogLevel::Normal.
	 * @return A shared reference to a TPreparedCliCommand representing the import operation.
	 */
	static TSharedRef<TPreparedCliCommand<>> Import(
		const FString& GameDataUrl,
		const FString& ApiKey,
		const TArray<FString>& SchemaNamesOrIds,
		const TSharedRef<FJsonObject>& DocumentsBySchemaNameOrId,
		const EImportMode ImportMode,
		const ECharonLogLevel LogsVerbosity = ECharonLogLevel::Normal
	);

	/**
	 * @brief Import documents from a file into a specified GameDataUrl file or server.
	 *
	 * @param GameDataUrl The URL of the GameData file or server.
	 * @param ApiKey Authentication credentials if GameDataUrl is a server, otherwise empty.
	 * @param SchemaNamesOrIds Names or IDs of schemas to import. Can be empty or '*' to import all documents.
	 * @param ImportMode The mode of import operation, see EImportMode for details.
	 * @param DocumentsBySchemaNameOrIdFilePath File path to the documents to import.
	 * @param Format The format of the imported documents ('json', 'bson', 'msgpack', 'xlsx', 'auto').
	 * @param LogsVerbosity The verbosity level of logs. Defaults to ECharonLogLevel::Normal.
	 * @return A shared reference to a TPreparedCliCommand representing the import operation.
	 */
	static TSharedRef<TPreparedCliCommand<>> ImportFromFile(
		const FString& GameDataUrl,
		const FString& ApiKey,
		const TArray<FString>& SchemaNamesOrIds,
		const EImportMode ImportMode,
		const FString& DocumentsBySchemaNameOrIdFilePath,
		const FString& Format = "json",
		const ECharonLogLevel LogsVerbosity = ECharonLogLevel::Normal
	);

	/**
	 * @brief Export documents from a GameDataUrl file or server.
	 *
	 * @param GameDataUrl The URL of the GameData file or server.
	 * @param ApiKey Authentication credentials if GameDataUrl is a server, otherwise empty.
	 * @param SchemaNamesOrIds Names or IDs of schemas to export. Can be empty or '*' to export all documents.
	 * @param Properties Names, IDs, types of properties in schemas to include in the export. Can be empty or '*' to export all properties.
	 * @param Languages Language tags (BCP 47) to include in the export of localized text. Can be empty or '*' to export all languages.
	 * @param ExportMode The mode of export operation, see EExportMode for details.
	 * @param LogsVerbosity The verbosity level of logs. Defaults to ECharonLogLevel::Normal.
	 * @return A shared reference to a TPreparedCliCommand<TSharedPtr<FJsonObject>> representing the export operation. The command, upon completion, provides the exported documents.
	 */
	static TSharedRef<TPreparedCliCommand<TSharedPtr<FJsonObject>>> Export(
		const FString& GameDataUrl,
		const FString& ApiKey,
		const TArray<FString>& SchemaNamesOrIds,
		const TArray<FString>& Properties,
		const TArray<FString>& Languages,
		const EExportMode ExportMode,
		const ECharonLogLevel LogsVerbosity = ECharonLogLevel::Normal
	);

	/**
	 * @brief Export documents from a GameDataUrl file or server to a file.
	 *
	 * @param GameDataUrl The URL of the GameData file or server.
	 * @param ApiKey Authentication credentials if GameDataUrl is a server, otherwise empty.
	 * @param SchemaNamesOrIds Names or IDs of schemas to export. Can be empty or '*' to export all documents.
	 * @param Properties Names, IDs, types of properties in schemas to include in the export. Can be empty or '*' to export all properties.
	 * @param Languages Language tags (BCP 47) to include in the export of localized text. Can be empty or '*' to export all languages.
	 * @param ExportMode The mode of export operation, see EExportMode for details.
	 * @param ExportedDocumentsFilePath File path where the exported documents will be saved.
	 * @param Format The format in which to save the exported data ('json', 'bson', 'msgpack', 'xlsx').
	 * @param LogsVerbosity The verbosity level of logs. Defaults to ECharonLogLevel::Normal.
	 * @return A shared reference to a TPreparedCliCommand<TSharedPtr<FJsonObject>> representing the export operation. The command, upon completion, provides the exported documents.
	 */
	static TSharedRef<TPreparedCliCommand<>> ExportToFile(
		const FString& GameDataUrl,
		const FString& ApiKey,
		const TArray<FString>& SchemaNamesOrIds,
		const TArray<FString>& Properties,
		const TArray<FString>& Languages,
		const EExportMode ExportMode,
		const FString& ExportedDocumentsFilePath,
		const FString& Format = "json",
		const ECharonLogLevel LogsVerbosity = ECharonLogLevel::Normal
	);

	/**
	 * @brief Import documents grouped by schema into a specified GameDataUrl file or server.
	 *
	 * @param GameDataUrl The URL of the GameData file or server.
	 * @param ApiKey Authentication credentials if GameDataUrl is a server, otherwise empty.
	 * @param SchemaNamesOrIds Names or IDs of schemas to import from DocumentsBySchemaNameOrId. Can be empty or '*' to import all documents.
	 * @param LanguageIds Language tags (BCP 47) to import into localized text. Can be empty or '*' to import all languages.
	 * @param DocumentsBySchemaNameOrId The documents to be imported, grouped by schema name or ID.
	 * @param LogsVerbosity The verbosity level of logs. Defaults to ECharonLogLevel::Normal.
	 * @return A shared reference to a TPreparedCliCommand representing the import operation.
	 */
	static TSharedRef<TPreparedCliCommand<>> I18NImport(
		const FString& GameDataUrl,
		const FString& ApiKey,
		const TArray<FString>& SchemaNamesOrIds,
		const TArray<FString>& LanguageIds,
		const TSharedRef<FJsonObject>& DocumentsBySchemaNameOrId,
		const ECharonLogLevel LogsVerbosity = ECharonLogLevel::Normal
	);

	/**
	 * @brief Import documents from a file into a specified GameDataUrl file or server.
	 *
	 * @param GameDataUrl The URL of the GameData file or server.
	 * @param ApiKey Authentication credentials if GameDataUrl is a server, otherwise empty.
	 * @param SchemaNamesOrIds Names or IDs of schemas to import. Can be empty or '*' to import all documents.
	 * @param LanguageIds Language tags (BCP 47) to import into localized text. Can be empty or '*' to import all languages.
	 * @param DocumentsBySchemaNameOrIdFilePath File path to the documents to import.
	 * @param Format The format of the imported documents ('json', 'bson', 'msgpack', 'xlsx', 'auto').
	 * @param LogsVerbosity The verbosity level of logs. Defaults to ECharonLogLevel::Normal.
	 * @return A shared reference to a TPreparedCliCommand representing the import operation.
	 */
	static TSharedRef<TPreparedCliCommand<>> I18NImportFromFile(
		const FString& GameDataUrl,
		const FString& ApiKey,
		const TArray<FString>& SchemaNamesOrIds,
		const TArray<FString>& LanguageIds,
		const FString& DocumentsBySchemaNameOrIdFilePath,
		const FString& Format = "json",
		const ECharonLogLevel LogsVerbosity = ECharonLogLevel::Normal
	);

	/**
	 * @brief Export documents from a GameDataUrl file or server.
	 *
	 * @param GameDataUrl The URL of the GameData file or server.
	 * @param ApiKey Authentication credentials if GameDataUrl is a server, otherwise empty.
	 * @param SchemaNamesOrIds Names or IDs of schemas to export. Can be empty or '*' to export all documents.
	 * @param SourceLanguage Language tag (BCP 47) to include in the export as source of translation.
	 * @param TargetLanguage Language tag (BCP 47) to include in the export as target of translation.
	 * @param LogsVerbosity The verbosity level of logs. Defaults to ECharonLogLevel::Normal.
	 * @return A shared reference to a TPreparedCliCommand<TSharedPtr<FJsonObject>> representing the export operation. The command, upon completion, provides the exported documents.
	 */
	static TSharedRef<TPreparedCliCommand<TSharedPtr<FJsonObject>>> I18NExport(
		const FString& GameDataUrl,
		const FString& ApiKey,
		const TArray<FString>& SchemaNamesOrIds,
		const FString& SourceLanguage,
		const FString& TargetLanguage,
		const ECharonLogLevel LogsVerbosity = ECharonLogLevel::Normal
	);

	/**
	 * @brief Export documents from a GameDataUrl file or server to a file.
	 *
	 * @param GameDataUrl The URL of the GameData file or server.
	 * @param ApiKey Authentication credentials if GameDataUrl is a server, otherwise empty.
	 * @param SchemaNamesOrIds Names or IDs of schemas to export. Can be empty or '*' to export all documents.
	 * @param SourceLanguage Language tag (BCP 47) to include in the export as source of translation.
	 * @param TargetLanguage Language tag (BCP 47) to include in the export as target of translation.
	 * @param ExportedDocumentsFilePath File path where the exported documents will be saved.
	 * @param Format The format in which to save the exported data ('json', 'bson', 'msgpack', 'xlsx').
	 * @param LogsVerbosity The verbosity level of logs. Defaults to ECharonLogLevel::Normal.
	 * @return A shared reference to a TPreparedCliCommand<TSharedPtr<FJsonObject>> representing the export operation. The command, upon completion, provides the exported documents.
	 */
	static TSharedRef<TPreparedCliCommand<>> I18NExportToFile(
		const FString& GameDataUrl,
		const FString& ApiKey,
		const TArray<FString>& SchemaNamesOrIds,
		const FString& SourceLanguage,
		const FString& TargetLanguage,
		const FString& ExportedDocumentsFilePath,
		const FString& Format = "json",
		const ECharonLogLevel LogsVerbosity = ECharonLogLevel::Normal
	);

	/**
	 * @brief Add translation language to a GameDataUrl file or server.
	 *
	 * @param GameDataUrl The URL of the GameData file or server.
	 * @param ApiKey Authentication credentials if GameDataUrl is a server, otherwise empty.
	 * @param LanguageIds Language tags (BCP 47) to add in project's translation language list.
	 * @param LogsVerbosity The verbosity level of logs. Defaults to ECharonLogLevel::Normal.
	 * @return A shared reference to a TPreparedCliCommand representing the import operation.
	 */
	static TSharedRef<TPreparedCliCommand<>> I18NAddLanguage(
		const FString& GameDataUrl,
		const FString& ApiKey,
		const TArray<FString>& LanguageIds,
		const ECharonLogLevel LogsVerbosity = ECharonLogLevel::Normal
	);
	
	/**
	 * @brief Compares all documents in two GameData URLs and creates a patch representing the difference.
	 *
	 * @param GameDataUrl1 The first GameData URL for comparison.
	 * @param GameDataUrl2 The second GameData URL for comparison.
	 * @param ApiKey Authentication credentials if the GameData URLs are servers, otherwise empty.
	 * @param LogsVerbosity The verbosity level of logs. Defaults to ECharonLogLevel::Normal.
	 * @return A shared reference to a TPreparedCliCommand<TSharedPtr<FJsonObject>> representing the create patch operation. The command, upon completion, provides the patch data.
	 */
	static TSharedRef<TPreparedCliCommand<TSharedPtr<FJsonObject>>> CreatePatch(
		const FString& GameDataUrl1,
		const FString& GameDataUrl2,
		const FString& ApiKey,
		const ECharonLogLevel LogsVerbosity = ECharonLogLevel::Normal
	);
	
	/**
	 * @brief Compares all documents in two GameData URLs and creates a patch representing the difference.
	 *
	 * @param GameDataUrl1 The first GameData URL for comparison.
	 * @param GameDataUrl2 The second GameData URL for comparison.
	 * @param ApiKey Authentication credentials if the GameData URLs are servers, otherwise empty.
	 * @param PathDocumentFilePath File path where the patch document will be saved.
	 * @param Format The format in which to save the patch data ('json', 'bson', 'msgpack').
	 * @param LogsVerbosity The verbosity level of logs. Defaults to ECharonLogLevel::Normal.
	 * @return A shared reference to a TPreparedCliCommand<TSharedPtr<FJsonObject>> representing the create patch operation. The command, upon completion, provides the patch data.
	 */
	static TSharedRef<TPreparedCliCommand<>> CreatePatchToFile(
		const FString& GameDataUrl1,
		const FString& GameDataUrl2,
		const FString& ApiKey,
		const FString& PathDocumentFilePath,
		const FString& Format = "json",
		const ECharonLogLevel LogsVerbosity = ECharonLogLevel::Normal
	);

	/**
	 * @brief Applies a patch created by CreatePatch to a specified GameData URL.
	 *
	 * @param GameDataUrl The GameData URL to apply the patch to.
	 * @param ApiKey Authentication credentials if GameDataUrl is a server, otherwise empty.
	 * @param GameDataPatch The patch document created by CreatePatch.
	 * @param LogsVerbosity The verbosity level of logs. Defaults to ECharonLogLevel::Normal.
	 * @return A shared reference to a TPreparedCliCommand representing the apply patch operation.
	 */
	static TSharedRef<TPreparedCliCommand<>> ApplyPatch(
		const FString& GameDataUrl,
		const FString& ApiKey,
		const TSharedRef<FJsonObject>& GameDataPatch,
		const ECharonLogLevel LogsVerbosity = ECharonLogLevel::Normal
	);

	/**
	 * @brief Applies a patch created by CreatePatch to a specified GameData URL.
	 *
	 * @param GameDataUrl The GameData URL to apply the patch to.
	 * @param ApiKey Authentication credentials if GameDataUrl is a server, otherwise empty.
	 * @param PathDocumentFilePath Patch file path created by CreatePatch command.
	 * @param Format The format of the patch documents ('json', 'bson', 'msgpack', 'auto').
	 * @param LogsVerbosity The verbosity level of logs. Defaults to ECharonLogLevel::Normal.
	 * @return A shared reference to a TPreparedCliCommand representing the apply patch operation.
	 */
	static TSharedRef<TPreparedCliCommand<>> ApplyPatchFromFile(
		const FString& GameDataUrl,
		const FString& ApiKey,
		const FString& PathDocumentFilePath,
		const FString& Format = "json",
		const ECharonLogLevel LogsVerbosity = ECharonLogLevel::Normal
	);

	/**
	 * @brief Backups game data with all documents and their metadata.
	 *
	 * @param GameDataUrl The GameData URL to backup.
	 * @param ApiKey Authentication credentials if GameDataUrl is a server, otherwise empty.
	 * @param LogsVerbosity The verbosity level of logs. Defaults to ECharonLogLevel::Normal.
	 * @return A shared reference to a TPreparedCliCommand<TSharedPtr<FJsonObject>> representing the backup operation. The command, upon completion, provides the backup data.
	 */
	static TSharedRef<TPreparedCliCommand<TSharedPtr<FJsonObject>>> Backup(
		const FString& GameDataUrl,
		const FString& ApiKey,
		const ECharonLogLevel LogsVerbosity = ECharonLogLevel::Normal
	);

	/**
	 * @brief Backups game data to a file with all documents and their metadata.
	 *
	 * @param GameDataUrl The GameData URL to backup.
	 * @param ApiKey Authentication credentials if GameDataUrl is a server, otherwise empty.
	 * @param GameDataFilePath File path where the backup will be saved.
	 * @param Format The format for saving data ('json', 'msgpack').
	 * @param LogsVerbosity The verbosity level of logs. Defaults to ECharonLogLevel::Normal.
	 * @return A shared reference to a TPreparedCliCommand representing the backup to file operation. The command, upon completion, provides the backup data.
	 */
	static TSharedRef<TPreparedCliCommand<>> BackupToFile(
		const FString& GameDataUrl,
		const FString& ApiKey,
		const FString& GameDataFilePath,
		const FString& Format = "json",
		const ECharonLogLevel LogsVerbosity = ECharonLogLevel::Normal
	);

	/**
	 * @brief Restores game data with all documents and their metadata.
	 *
	 * @param GameDataUrl The GameData URL to restore.
	 * @param ApiKey Authentication credentials if GameDataUrl is a server, otherwise empty.
	 * @param GameData Previously backed up data.
	 * @param LogsVerbosity The verbosity level of logs. Defaults to ECharonLogLevel::Normal.
	 * @return A shared reference to a TPreparedCliCommand representing the restore operation.
	 */
	static TSharedRef<TPreparedCliCommand<>> Restore(
		const FString& GameDataUrl,
		const FString& ApiKey,
		const TSharedRef<FJsonObject>& GameData,
		const ECharonLogLevel LogsVerbosity = ECharonLogLevel::Normal
	);

	/**
	 * @brief Restores game data from a file with all documents and their metadata.
	 *
	 * @param GameDataUrl The GameData URL to restore.
	 * @param ApiKey Authentication credentials if GameDataUrl is a server, otherwise empty.
	 * @param GameDataFilePath File path with previously backed up data.
	 * @param Format The format for the backed up data ('json', 'msgpack', 'auto').
	 * @param LogsVerbosity The verbosity level of logs. Defaults to ECharonLogLevel::Normal.
	 * @return A shared reference to a TPreparedCliCommand representing the restore from file operation.
	 */
	static TSharedRef<TPreparedCliCommand<>> RestoreFromFile(
		const FString& GameDataUrl,
		const FString& ApiKey,
		const FString& GameDataFilePath,
		const FString& Format = "json",
		const ECharonLogLevel LogsVerbosity = ECharonLogLevel::Normal
	);

	/**
	 * @brief Checks all documents in the specified GameData URL and returns a report with any issues.
	 *
	 * @param GameDataUrl The GameData URL to validate.
	 * @param ApiKey Authentication credentials if GameDataUrl is a server, otherwise empty.
	 * @param ValidationOptions A list of checks to perform during validation, see EValidationOption for details.
	 * @param LogsVerbosity The verbosity level of logs. Defaults to ECharonLogLevel::Normal.
	 * @return A shared reference to a TPreparedCliCommand representing the validation operation. The command, upon completion, provides the report of issues.
	 */
	static TSharedRef<TPreparedCliCommand<FValidationReport>> Validate(
		const FString& GameDataUrl,
		const FString& ApiKey,
		const TArray<EValidationOption>& ValidationOptions,
		const ECharonLogLevel LogsVerbosity = ECharonLogLevel::Normal
	);

	/**
	 * @brief Generates C++ source code for loading game data from a GameDataUrl into a game's runtime.
	 *
	 * @param GameDataUrl The GameData URL from which to generate source code.
	 * @param ApiKey Authentication credentials if GameDataUrl is a server, otherwise empty.
	 * @param OutputDirectory Directory to place generated files, preferably empty.
	 * @param DocumentClassName Name for the base class for all documents.
	 * @param GameDataClassName Name for the main class from which all documents are accessible.
	 * @param DefineConstants Additional defines for all generated files.
	 * @param SourceCodeGenerationOptimizations List of enabled optimizations in the generated code, see ESourceCodeGenerationOptimizations for details.
	 * @param SourceCodeIndentation Indentation style for the generated code.
	 * @param SourceCodeLineEndings Line endings for the generated code.
	 * @param ClearOutputDirectory Whether to clear the output directory from generated code before generating files.
	 * @param LogsVerbosity The verbosity level of logs. Defaults to ECharonLogLevel::Normal.
	 * @return A shared reference to a TPreparedCliCommand representing the source code generation operation.
	 */
	static TSharedRef<TPreparedCliCommand<>> GenerateUnrealEngineSourceCode(
		const FString& GameDataUrl,
		const FString& ApiKey,
		const FString& OutputDirectory,
		const FString& DocumentClassName = "UDocument",
		const FString& GameDataClassName = "UGameData",
		const FString& DefineConstants = FString(),
		const TArray<ESourceCodeGenerationOptimizations>& SourceCodeGenerationOptimizations = TArray<ESourceCodeGenerationOptimizations>(),
		const ESourceCodeIndentation SourceCodeIndentation = ESourceCodeIndentation::Tabs,
#if PLATFORM_WINDOWS
		const ESourceCodeLineEndings SourceCodeLineEndings = ESourceCodeLineEndings::Windows,
#else
		const ESourceCodeLineEndings SourceCodeLineEndings = ESourceCodeLineEndings::Unix,
#endif
		const bool ClearOutputDirectory = true,
		const ECharonLogLevel LogsVerbosity = ECharonLogLevel::Normal
	);

	/**
	 * @brief Dumps T4 code generation templates used to generate source code into a specified directory.
	 *
	 * @param OutputDirectory The directory where the templates will be dumped.
	 * @return A shared reference to a TPreparedCliCommand representing the template dumping operation.
	 */
	static TSharedRef<TPreparedCliCommand<>> DumpTemplates(
		const FString& OutputDirectory
	);

	/**
	 * @brief Gets the version number of the Charon.exe executable.
	 *
	 * @return A shared reference to a TPreparedCliCommand containing the version number as a string.
	 */
	static TSharedRef<TPreparedCliCommand<FString>> GetVersion();

	/**
	 * @brief Gets the version of the Charon.exe executable used to create the specified GameData URL.
	 *
	 * @param GameDataUrl The GameData URL to check.
	 * @param ApiKey Authentication credentials if GameDataUrl is a server, otherwise empty.
	 * @param LogsVerbosity The verbosity level of logs. Defaults to ECharonLogLevel::Normal.
	 * @return A shared reference to a TPreparedCliCommand containing the version number as a string.
	 */
	static TSharedRef<TPreparedCliCommand<FString>> GetGameDataVersion(
		const FString& GameDataUrl,
		const FString& ApiKey,
		const ECharonLogLevel LogsVerbosity = ECharonLogLevel::Normal
	);

	/**
	 * @brief Init the specified GameData file.
	 *
	 * @param GameDataPath The path of the GameData file.
	 * @param LogsVerbosity The verbosity level of logs. Defaults to ECharonLogLevel::Normal.
	 * @return A shared reference to a TPreparedCliCommand containing the version number as a string.
	 */
	static TSharedRef<TPreparedCliCommand<>> InitGameData(
		const FString& GameDataPath,
		const ECharonLogLevel LogsVerbosity = ECharonLogLevel::Normal
	);

	/**
	 * @brief Run specified command with charon tool.
	 *
	 * @param CommandsAndOptions List of commands and options to pass to charon tool.
	 * @param ApiKey Authentication credentials if GameDataUrl is a server, otherwise empty.
	 * @return A shared reference to a TPreparedCliCommand which could be run when needed.
	 */
	static TSharedRef<TPreparedCliCommand<FString>> RunCharon(
		const TArray<FString>& CommandsAndOptions,
		const FString& ApiKey
	);

	/**
	 * @brief Run dotnet-t4 command-line tool for processing T4 templates. It is a general-purpose way to generate text or code files using C#.
	 * https://github.com/mono/t4/blob/main/dotnet-t4/readme.md
	 * @param TemplateFile The path of the template .tt file.
	 * @param ReferencedAssemblies An assembly reference by path or assembly name. It will be resolved from the framework and assembly directories.
	 * @param Usings A namespace imports which generate a using statement in template source code.
	 * @param IncludeDirectories A directory to be searched when resolving included files.
	 * @param AssemblyLookupDirectories A directory to be searched when resolving assemblies.
	 * @param Parameters session parameter Pair{FString,FString}.Key toPair{FString,FString}.Value.
	 * @param bUseRelativeLinePragmas Use relative paths in line pragmas.
	 * @param bDebugMode Generate debug symbols and keep temporary files.
	 * @param bVerboseLogs >Output additional diagnostic information to stdout.
	 * @return A shared reference to a TPreparedCliCommand which could be run when needed.
	 */
	static TSharedRef<TPreparedCliCommand<FString>> RunT4(
		const FString& TemplateFile,
		const TArray<FString>& ReferencedAssemblies = TArray<FString>(),
		const TArray<FString>& Usings = TArray<FString>(),
		const TArray<FString>& IncludeDirectories = TArray<FString>(),
		const TArray<FString>& AssemblyLookupDirectories = TArray<FString>(),
		const TArray<TPair<FString, FString>>& Parameters = TArray<TPair<FString, FString>>(),
		const bool bUseRelativeLinePragmas = false,
		const bool bDebugMode = false,
		const bool bVerboseLogs = false
	);

	/**
	 * @brief Run dotnet-t4 command-line tool for processing T4 templates. It is a general-purpose way to generate text or code files using C#.
	 * https://github.com/mono/t4/blob/main/dotnet-t4/readme.md
	 * @param TemplateFile The path of the template .tt file.
	 * @param OutputFile The name or path of the output file. It defaults to the input filename with its extension changed to .txt, or to match the generated code when preprocessing, and may be overridden by template settings. Use - instead of a filename to write to stdout.
	 * @param Usings A namespace imports which generate a using statement in template source code.
	 * @param TemplateClassName Preprocess the template into class name for use as a runtime template. The class name may include a namespace.
	 * @param bUseRelativeLinePragmas Use relative paths in line pragmas.
	 * @param bDebugMode Generate debug symbols and keep temporary files.
	 * @param bVerboseLogs >Output additional diagnostic information to stdout.
	 * @return A shared reference to a TPreparedCliCommand which could be run when needed.
	 */
	static TSharedRef<TPreparedCliCommand<FString>> PreprocessT4(
		const FString& TemplateFile,
		const FString& OutputFile,
		const FString& TemplateClassName,
		const TArray<FString>& Usings = TArray<FString>(),
		const bool bUseRelativeLinePragmas = false,
		const bool bDebugMode = false,
		const bool bVerboseLogs = false
	);
	
private:
	static FString GetLogOptions(const ECharonLogLevel LogsVerbosity);
	static FString WriteJsonToTempFile(const TSharedRef<FJsonObject>& JsonObject);
	static FString PrepareTempOutputFile(const FString& Extension = TEXT(".tmp"));
};