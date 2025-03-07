// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "FBranch.generated.h"

USTRUCT()
struct FBranch
{
	GENERATED_BODY();

	/** Id of branch, also it is dataSourceId. */
	UPROPERTY()
	FString Id;

	/** Name of branch. */
	UPROPERTY()
	FString Name;

	/** Is it primary branch. */
	UPROPERTY()
	bool IsPrimary = false;

	/** Size of branch. */
	UPROPERTY()
	int32 DataSize = 0;
};