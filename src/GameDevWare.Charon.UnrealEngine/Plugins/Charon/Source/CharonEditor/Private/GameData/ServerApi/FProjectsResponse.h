// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "FProject.h"
#include "FApiError.h"

#include "FProjectsResponse.generated.h"

USTRUCT()
struct FProjectsResponse
{
	GENERATED_BODY();
	
	/** List of projects. */
	UPROPERTY()
	TArray<FProject> Result;

	/** Any error occurred during request. Null if no errors occurred. */
	UPROPERTY()
	TArray<FApiError> Errors;
};