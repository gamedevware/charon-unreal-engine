// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "FImportReportDocumentChange.h"

#include "FImportReport.generated.h"

USTRUCT()
struct CHARONEDITOR_API FImportReport
{
	GENERATED_BODY();
	
	/** List of all changes made during import. */
	UPROPERTY(VisibleAnywhere, Category="Game Data")
	TArray<FImportReportDocumentChange> Changes;

	/** Hash of metadata after import operation. */
	UPROPERTY(VisibleAnywhere, Category="Game Data")
	FString MetadataHash;
	
	/** Hash of game data after import operation. */
	UPROPERTY(VisibleAnywhere, Category="Game Data")
	FString RevisionHash;
};