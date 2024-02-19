// Copyright GameDevWare, Denis Zykov 2024

#pragma once

#include "FProject.generated.h"

USTRUCT()
struct FProject
{
	GENERATED_BODY();

	/** Id of project. */
	UPROPERTY()
	FString Id;

	/** Name of project. */
	UPROPERTY()
	FString Name;

	/** Picture URL of project. */
	UPROPERTY()
	FString PictureUrl;

	/** Picture URL of project. */
	UPROPERTY()
	TArray<FBranch> Branches;
};