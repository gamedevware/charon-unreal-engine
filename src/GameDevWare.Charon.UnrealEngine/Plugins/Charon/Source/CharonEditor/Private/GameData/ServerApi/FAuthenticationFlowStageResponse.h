// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "FAuthenticationFlowStage.h"
#include "FApiError.h"

#include "FAuthenticationFlowStageResponse.generated.h"

USTRUCT()
struct FAuthenticationFlowStageResponse
{
	GENERATED_BODY();
	
	/** List of projects. */
	UPROPERTY()
	FAuthenticationFlowStage Result;

	/** Any error occurred during request. Null if no errors occurred. */
	UPROPERTY()
	TArray<FApiError> Errors;
};