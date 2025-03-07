// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "FDeferredGameDataImportRecord.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFDeferredGameDataImporter, Log, All);

class FDeferredGameDataImporter
{
private:
	static FDeferredGameDataImporter* SingletonInstance;

	static void WriteJsonToImportFile(const FDeferredGameDataImportRecord& ImportRecord);
	static void ReadJsonFromImportFiles(TArray<FDeferredGameDataImportRecord>& OutImportRecords);
	
public:
	static void RegisterGameDataToImport(const FString& FileName, const FString& ModuleName, const FString& ClassName);
	static void ContinueDeferredImports();
	static bool TryToImportGameData(const FString& FileName, const FString& ModuleName, const FString& ClassName);
};
