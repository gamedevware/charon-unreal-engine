#pragma once

UENUM(BlueprintType)
enum class EStringComparison : uint8 {
	/*
	 * Compare strings using culture-sensitive sort rules and the current culture.
	 */
	CurrentCulture = 0,
	/*
	 * Compare strings using culture-sensitive sort rules, the current culture, and ignoring the case of the strings being compared.
	 */
	CurrentCultureIgnoreCase = 1,
	/*
	 * Compare strings using culture-sensitive sort rules and the invariant culture.
	 */
	InvariantCulture = 2,
	/*
	 * Compare strings using culture-sensitive sort rules, the invariant culture, and ignoring the case of the strings being compared.
	 */
	InvariantCultureIgnoreCase = 3,
	/*
	 * Compare strings using ordinal (binary) sort rules.
	 */
	Ordinal = 4,
	/*
	 * Compare strings using ordinal (binary) sort rules and ignoring the case of the strings being compared.
	 */
	OrdinalIgnoreCase = 5
};