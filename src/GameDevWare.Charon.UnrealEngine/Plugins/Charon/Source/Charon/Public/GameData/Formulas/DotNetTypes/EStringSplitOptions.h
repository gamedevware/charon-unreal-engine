#pragma once

#include "CoreMinimal.h"
#include "EStringSplitOptions.generated.h"

UENUM(BlueprintType)
enum class EStringSplitOptions : uint8 {
	/*
	 * The return value includes array elements that contain an empty string
	 */
	None = 0,
	/*
	 * The return value does not include array elements that contain an empty string
	 */
	RemoveEmptyEntries = 1,
	/*
	 * The return elements are trimmed.
	 */
	TrimEntries = 2
};