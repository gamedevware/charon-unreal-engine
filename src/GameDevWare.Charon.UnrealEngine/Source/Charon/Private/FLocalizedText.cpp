// Copyright GameDevWare, Denis Zykov 2024

#pragma once

#include "GameData/FLocalizedText.h"

FLocalizedText::FLocalizedText
(
) : CurrentValueLanguageId()
    , Current(FText::GetEmpty())
    , LanguageSwitcher(nullptr)
    , TextByLanguageId()
{
}

FLocalizedText::FLocalizedText
(
	const FLocalizedText& Other
) : CurrentValueLanguageId(Other.CurrentValueLanguageId)
    , Current(Other.Current)
    , LanguageSwitcher(Other.LanguageSwitcher)
    , TextByLanguageId(Other.TextByLanguageId)
{
}

FLocalizedText::FLocalizedText
(
	const TMap<FString, FText>& TextByLanguageId,
	ULanguageSwitcher* LanguageSwitcher
) : CurrentValueLanguageId()
    , Current(FText::GetEmpty())
    , LanguageSwitcher(LanguageSwitcher)
    , TextByLanguageId(TextByLanguageId)
{
}

FText FLocalizedText::GetCurrent()
{
	if (LanguageSwitcher == nullptr)
	{
		return FText::GetEmpty();
	}

	const auto CurrentLanguageId = LanguageSwitcher->CurrentLanguageId;
	if (CurrentLanguageId == CurrentValueLanguageId)
	{
		return Current;
	}

	const FText* FoundText;
	switch (LanguageSwitcher->FallbackOption)
	{
	case ELocalizedStringFallbackOption::OnNull:
	case ELocalizedStringFallbackOption::OnNotDefined:
		FoundText = TextByLanguageId.Find(LanguageSwitcher->CurrentLanguageId);
		if (FoundText != nullptr)
		{
			Current = *FoundText;
			break;
		}
		FoundText = TextByLanguageId.Find(LanguageSwitcher->FallbackLanguageId);
		if (FoundText != nullptr)
		{
			Current = *FoundText;
			break;
		}
		FoundText = TextByLanguageId.Find(LanguageSwitcher->PrimaryLanguageId);
		if (FoundText != nullptr)
		{
			Current = *FoundText;
			break;
		}
		Current = FText::GetEmpty();
		break;
	default:
	case ELocalizedStringFallbackOption::OnEmpty:
		CurrentValueLanguageId = CurrentLanguageId;
		FoundText = TextByLanguageId.Find(LanguageSwitcher->CurrentLanguageId);
		if (FoundText != nullptr && !FoundText->IsEmpty())
		{
			Current = *FoundText;
			break;
		}

		FoundText = TextByLanguageId.Find(LanguageSwitcher->FallbackLanguageId);
		if (FoundText != nullptr && !FoundText->IsEmpty())
		{
			Current = *FoundText;
			break;
		}

		FoundText = TextByLanguageId.Find(LanguageSwitcher->PrimaryLanguageId);
		if (FoundText != nullptr && !FoundText->IsEmpty())
		{
			Current = *FoundText;
			break;
		}

		Current = FText::GetEmpty();
		break;
	}

	return Current;
}
