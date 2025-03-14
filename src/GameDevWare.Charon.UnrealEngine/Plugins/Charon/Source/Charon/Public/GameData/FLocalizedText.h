// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "CoreTypes.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Containers/UnrealString.h"
#include "Templates/SharedPointer.h"
#include "Internationalization/Text.h"
#include "ULanguageSwitcher.h"

#include "FLocalizedText.generated.h"

/*
 * Collection of translated text by BCP-47 language tag.
 * Internally it is just Map of FText by language tag.
 * To change value returned by GetCurrent() use SetLanguage() method on your UGameDataBase-derived class.
 */
USTRUCT(BlueprintType)
struct CHARON_API FLocalizedText
{
	GENERATED_BODY()

private:
	FString CurrentValueLanguageId;
	FText Current;
	
	UPROPERTY()
	ULanguageSwitcher* LanguageSwitcher;
	
public:
	FLocalizedText();
	FLocalizedText(const FLocalizedText& Other);
	FLocalizedText(const TMap<FString, FText>& TextByLanguageId,
					 ULanguageSwitcher* LanguageSwitcher);

	/*
	 * Map of FText by BCP-47 language tag. Values could be empty FTexts. Not all supported language tags could present in map.
	 */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Game Data Localization")
	TMap<FString, FText> TextByLanguageId;

	/*
	 * Get FText of currently selected language. Use SetLanguage() method on your UGameDataBase-derived class to change current FText for whole game data.
	 */
	FText GetCurrent();
};

