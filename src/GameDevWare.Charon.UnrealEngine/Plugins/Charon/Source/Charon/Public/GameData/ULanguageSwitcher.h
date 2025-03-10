// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "CoreTypes.h"
#include "ELocalizedTextFallbackOption.h"
#include "UObject/Class.h"

#include "ULanguageSwitcher.generated.h"

/*
 * Internally used class to switch and track current selected language for UGameDataBase-derived class.
 */
UCLASS(BlueprintType)
class CHARON_API ULanguageSwitcher : public UObject
{
	GENERATED_BODY()

public:
	/*
	 * Currently selected language. Value is BCP-47 language tag.
	 */
	UPROPERTY(BlueprintReadOnly, Category="Game Data")
	FString CurrentLanguageId = "en-US";
	/*
	 * Fallback language which is used is value of CurrentLanguageId is empty string. Value is BCP-47 language tag.
	 */
	UPROPERTY(BlueprintReadOnly, Category="Game Data")
	FString FallbackLanguageId = "en-US";
	/*
	 * Primary language of UGameDataBase which is set in Project Settings which is used if FallbackLanguageId is empty string. Value is BCP-47 language tag.
	 */
	UPROPERTY(BlueprintReadOnly, Category="Game Data")
	FString PrimaryLanguageId = "en-US";
	/*
	 *  When to fallback to `FallbackLanguageId` localization text.
	 */
	UPROPERTY(BlueprintReadOnly, Category="Game Data")
	ELocalizedTextFallbackOption FallbackOption = ELocalizedTextFallbackOption::OnNull;
};

