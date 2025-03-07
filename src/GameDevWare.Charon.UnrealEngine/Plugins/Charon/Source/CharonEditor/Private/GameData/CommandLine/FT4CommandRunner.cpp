// Copyright GameDevWare, Denis Zykov 2024

#pragma once

#include "GameData/CommandLine/FT4CommandRunner.h"

#include "FChmodProcess.h"
#include "GameData/CommandLine/FCharonCliCommandRunner.h"

DEFINE_LOG_CATEGORY(LogFT4CommandRunner);

FT4CommandRunner::FT4CommandRunner(FString InParameters)
	: FMonitoredProcess("", "", true, true), FileCleanupList(), EnvironmentVariables()
{
	WorkingDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	const FString CharonIntermediateDirectory = FCharonCliCommandRunner::GetOrCreateCharonIntermediateDirectory();

#if PLATFORM_WINDOWS
	RunScriptPath = CharonIntermediateDirectory / TEXT("RunT4.bat");
	URL = TEXT("cmd.exe");
	Params = FString::Printf(TEXT("/c \"\"%s\" %s\""), *RunScriptPath, *InParameters);
#else
	RunScriptPath = CharonIntermediateDirectory / TEXT("RunT4.sh");
	URL = TEXT("/usr/bin/env");
	Params = FString::Printf(TEXT(" -- \"%s\" %s"), *RunScriptPath, *InParameters);
#endif
}

FT4CommandRunner::~FT4CommandRunner()
{
	IFileManager& FileManager = IFileManager::Get();
	for (FString FileName : FileCleanupList)
	{
		const bool bDeleted = FileManager.Delete(*FileName, /* require exists */ false, /* even read only */ true,
		                                         /* quiet */ true);
		if (!bDeleted)
		{
			UE_LOG(LogFT4CommandRunner, Log,
			       TEXT("Failed to delete temporary filename '%s' for command '%s'."), *FileName, *Params);
		}
	}
}

bool FT4CommandRunner::Launch()
{
	for (auto KeyValue : EnvironmentVariables)
	{
		FPlatformMisc::SetEnvironmentVar(*KeyValue.Key, *KeyValue.Value);
	}

	UE_LOG(LogFT4CommandRunner, Log, TEXT("Launching CLI process with '%s %s'."), *URL, *Params);

	return FMonitoredProcess::Launch();
}

void FT4CommandRunner::AttachTemporaryFile(const FString& InFilePath)
{
	if (InFilePath.IsEmpty())
	{
		return;
	}
	FileCleanupList.Add(InFilePath);
}
