// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "ELocalizedTextFallbackOption.generated.h"

UENUM(BlueprintType)
/**
 * When to fallback to alternative localization text.
 */
enum class ELocalizedTextFallbackOption : uint8
{
	/**
	 * Fallback when the localized text is null (default)
	 */
	OnNull,
	/**
	 * Fallback when the localized text is an empty string
	 */
	OnEmpty,
	/**
	 * Fallback when the localized text is not defined in the LocalizedString
	 */
	OnNotDefined,
};

