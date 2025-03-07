// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "EImportReportDocumentChangeStatus.h"
#include "GameData/CommandLine/FDocumentValidationError.h"
#include "FImportReportDocumentChange.generated.h"

USTRUCT()
struct CHARONEDITOR_API FImportReportDocumentChange
{
	GENERATED_BODY();
	
	/** Id of the changed document. */
	UPROPERTY(VisibleAnywhere, Category="Game Data")
	FString Id;

	/** New id of the newly created document. */
	UPROPERTY(VisibleAnywhere, Category="Game Data")
	FString NewId;

	/** Schema name of changed document. */
	UPROPERTY(VisibleAnywhere, Category="Game Data")
	FString SchemaName;
	
	/** Schema name of changed document. */
	UPROPERTY(VisibleAnywhere, Category="Game Data")
	FString SchemaId;

	/** Status of change for document. */
	UPROPERTY(VisibleAnywhere, Category="Game Data")
	EImportReportDocumentChangeStatus Status = EImportReportDocumentChangeStatus::Skipped;

	/** Reason or informational comment for a change status. */
	UPROPERTY(VisibleAnywhere, Category="Game Data")
	FString Comment;

	/** List of errors happened during attempt to apply change. Could be empty even if EImportReportDocumentChangeStatus::Error */
	UPROPERTY(VisibleAnywhere, Category="Game Data")
	TArray<FDocumentValidationError> Errors;
};

