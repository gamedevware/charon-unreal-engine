﻿// Copyright GameDevWare, Denis Zykov 2024

#pragma once

#include "FGameDataDocumentReference.h"
#include "EDocumentReferenceGetResult.h"
#include "FLocalizedString.h"
#include "UGameDataDocument.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "UGameDataLibrary.generated.h"

/*
 * Additional function for Blueprints related to game data.
 */
UCLASS()
class UGameDataLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/*
	 * Get UGameDataDocument referenced in specified DocumentReference.
	 */
	UFUNCTION(BlueprintCallable, Meta = (ExpandEnumAsExecs = "Branches"), Category="Game Data")
	static UGameDataDocument* GetDocument(FGameDataDocumentReference DocumentReference, EDocumentReferenceGetResult& Branches)
	{
		UGameDataDocument* Document = DocumentReference.GetReferencedDocument();
		if (Document == nullptr)
		{
			Branches = EDocumentReferenceGetResult::NotFound;
		}
		else
		{
			Branches = EDocumentReferenceGetResult::Found;
		}
		return Document;
	}

	/*
	 * Get current text specified in internal ULanguageSwitcher of selected LocalizedString.
	 */
	UFUNCTION(BlueprintCallable, Category="Game Data Localization")
	static FText GetCurrentText(FLocalizedString LocalizedString)
	{
		return LocalizedString.GetCurrent();
	}
};
