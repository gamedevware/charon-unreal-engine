// Copyright GameDevWare, Denis Zykov 2025

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
