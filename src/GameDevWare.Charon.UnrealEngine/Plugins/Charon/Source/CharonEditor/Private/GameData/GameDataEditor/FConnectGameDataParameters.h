// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "GameData/ServerApi/FProject.h"
#include "GameData/ServerApi/FBranch.h"

struct CHARONEDITOR_API FConnectGameDataParameters
{
public:
	TSharedRef<FProject> Project;
	TSharedRef<FBranch> Branch;
	FString ApiKey;
	FString ServerAddress;
	int32 InitialSyncDirection; // 1 -> Upload, -1 Download
};
