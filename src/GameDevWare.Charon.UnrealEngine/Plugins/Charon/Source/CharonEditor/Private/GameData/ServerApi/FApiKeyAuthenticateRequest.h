// Copyright GameDevWare, Denis Zykov 2025

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
