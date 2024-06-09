#pragma once

#include "FDeferredGameDataImport.h"

class FLazyGameDataImporter
{
private:
	void WriteJsonToImportFile(const FDeferredGameDataImport& ImportRecord);
	void ReadJsonFromImportFiles(TArray<FDeferredGameDataImport>& OutImportRecords);
	
public:
	void RegisterDeferredImport(const FString& Filename, const FName& ClassName);
	void ContinueDeferredImport();
};
