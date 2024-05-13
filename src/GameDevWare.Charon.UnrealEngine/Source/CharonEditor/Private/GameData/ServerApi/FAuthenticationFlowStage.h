// Copyright GameDevWare, Denis Zykov 2024

#pragma once

#include "FAuthenticationFlowStage.generated.h"

USTRUCT()
struct FAuthenticationFlowStage
{
	GENERATED_BODY();

	/** Authorization code value. */
	UPROPERTY()
	FString AuthorizationCode;
};
