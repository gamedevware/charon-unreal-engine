// Copyright GameDevWare, Denis Zykov 2024

#pragma once

#include "FApiKeyAuthenticateRequest.generated.h"

USTRUCT()
struct FApiKeyAuthenticateRequest
{
	GENERATED_BODY();

	/** API Key value. */
	UPROPERTY()
	FString ApiKey;
};
