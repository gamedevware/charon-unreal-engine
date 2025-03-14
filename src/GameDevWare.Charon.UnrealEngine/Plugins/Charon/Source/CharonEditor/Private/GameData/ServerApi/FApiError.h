﻿// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "FApiError.generated.h"

USTRUCT()
struct FApiError
{
	GENERATED_BODY();
	
	/** Error message. */
	UPROPERTY()
	FString Message;

	/** Error code. */
	UPROPERTY()
	FString Code;
};