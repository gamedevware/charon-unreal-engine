// Copyright GameDevWare, Denis Zykov 2024

#pragma once

#include "GameData/CommandLine/FCharonCliCommandRunner.h"

#include "FChmodProcess.h"

DEFINE_LOG_CATEGORY(LogFCharonCliCommandRunner);

FCharonCliCommandRunner::FCharonCliCommandRunner(FString InParameters)
	: FMonitoredProcess("", "", true, true), FileCleanupList(), EnvironmentVariables()
{
	WorkingDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	const FString CharonIntermediateDirectory = FCharonCliCommandRunner::GetOrCreateCharonIntermediateDirectory();

#if PLATFORM_WINDOWS
	RunScriptPath = CharonIntermediateDirectory / TEXT("RunCharon.bat");
	URL = TEXT("cmd.exe");
	Params = FString::Printf(TEXT("/c \"\"%s\" %s\""), *RunScriptPath, *InParameters);
#elif PLATFORM_MAC || PLATFORM_LINUX
	RunScriptPath = CharonIntermediateDirectory / TEXT("RunCharon.sh");
	URL = TEXT("/usr/bin/env");
	Params = FString::Printf(TEXT(" -- \"%s\" %s"), *RunScriptPath, *InParameters);
#endif
}

FCharonCliCommandRunner::~FCharonCliCommandRunner()
{
	IFileManager& FileManager = IFileManager::Get();
	for (FString FileName : FileCleanupList)
	{
		const bool bDeleted = FileManager.Delete(*FileName, /* require exists */ false, /* even read only */ true,
		                                         /* quiet */ true);
		if (!bDeleted)
		{
			UE_LOG(LogFCharonCliCommandRunner, Log,
			       TEXT("Failed to delete temporary filename '%s' for command '%s'."), *FileName, *Params);
		}
	}
}

bool FCharonCliCommandRunner::Launch()
{
	for (auto KeyValue : EnvironmentVariables)
	{
		FPlatformMisc::SetEnvironmentVar(*KeyValue.Key, *KeyValue.Value);
	}

	UE_LOG(LogFCharonCliCommandRunner, Log, TEXT("Launching CLI process with '%s %s'."), *URL, *Params);

	return FMonitoredProcess::Launch();
}

void FCharonCliCommandRunner::SetApiKey(const FString& InApiKey)
{
	if (InApiKey.IsEmpty())
	{
		return;
	}
	this->EnvironmentVariables.Add(TEXT("CHARON_API_KEY"), InApiKey);
}

void FCharonCliCommandRunner::AttachTemporaryFile(const FString& InFilePath)
{
	if (InFilePath.IsEmpty())
	{
		return;
	}
	FileCleanupList.Add(InFilePath);
}

FString FCharonCliCommandRunner::GetOrCreateCharonIntermediateDirectory()
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	const FString PluginScriptsDirectory = FPaths::ConvertRelativePathToFull(
		FPaths::ProjectDir() / TEXT("Plugins/Charon/Resources/Scripts"));
	const FString CharonIntermediateDirectory = FPaths::ConvertRelativePathToFull(
		FPaths::ProjectIntermediateDir() / "Charon");
	if (!PlatformFile.DirectoryExists(*CharonIntermediateDirectory))
	{
		if (!PlatformFile.CreateDirectory(*CharonIntermediateDirectory))
		{
			UE_LOG(LogFCharonCliCommandRunner, Warning,
			       TEXT("Create directory '%s' in Project's intermediate directory."), *CharonIntermediateDirectory);
			return CharonIntermediateDirectory;
		}
		bScriptsCopied = false;
	}

	if (!bScriptsCopied)
	{
		bScriptsCopied = true;
		TArray<FString> FoundFiles;
		PlatformFile.FindFiles(FoundFiles, *PluginScriptsDirectory, nullptr);
		for (const FString SourceFilePath : FoundFiles)
		{
			const FString TargetFileName = CharonIntermediateDirectory / FPaths::GetCleanFilename(SourceFilePath);
			/*if (PlatformFile.FileExists(*TargetFileName))
			{
				continue;
			}*/
			if (!PlatformFile.CopyFile(*TargetFileName, *SourceFilePath))
			{
				UE_LOG(LogFCharonCliCommandRunner, Warning,
				       TEXT("Failed to copy file '%s' to Project's intermediate directory."), *SourceFilePath);
			}

#if PLATFORM_UNIX || PLATFORM_MAC || PLATFORM_LINUX
			if (FPaths::GetExtension(TargetFileName) == TEXT("sh") ||
				FPaths::GetExtension(TargetFileName) == TEXT("command"))
			{
				UE_LOG(LogFCharonCliCommandRunner, Log, TEXT("Running chmod +x for a script file '%s'."), *SourceFilePath);
				FChmodProcess ChmodProcess(TargetFileName, "+x");
				if(!ChmodProcess.Launch())
				{
					UE_LOG(LogFCharonCliCommandRunner, Error, TEXT("Failed to run chmod +x for a script file '%s'."), *SourceFilePath);
					continue;
				}
				while (ChmodProcess.Update())
				{
					FPlatformProcess::Sleep(0.01f);
				}
				if (ChmodProcess.GetReturnCode() != 0)
				{
					UE_LOG(LogFCharonCliCommandRunner, Error, TEXT("Process chmod +x for a script file '%s' exited with code %d."), *SourceFilePath, ChmodProcess.GetReturnCode());
				}
			}
#endif
		}
	}

	return CharonIntermediateDirectory;
}
