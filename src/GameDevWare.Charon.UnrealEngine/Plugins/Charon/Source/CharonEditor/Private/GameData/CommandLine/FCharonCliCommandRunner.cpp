// Copyright GameDevWare, Denis Zykov 2025

#include "GameData/CommandLine/FCharonCliCommandRunner.h"
#include "Interfaces/IPluginManager.h"

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
#else
	RunScriptPath = CharonIntermediateDirectory / TEXT("RunCharon.sh");
	URL = TEXT("/usr/bin/env");
	Params = FString::Printf(TEXT(" -- \"%s\" %s"), *RunScriptPath, *InParameters);
#endif

	const FString ContentRootDirectory = FCharonCliCommandRunner::GetOrCreateCharonContentRootDirectory();
	EnvironmentVariables.Add(TEXT("DOTNET_CONTENTROOT"), ContentRootDirectory);
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
	const FString CharonIntermediateDirectory = FPaths::ConvertRelativePathToFull(
		FPaths::ProjectIntermediateDir() / "Charon");

	const auto PluginName = TEXT("Charon");
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(PluginName);
	if(!Plugin.IsValid())
	{
		UE_LOG(LogFCharonCliCommandRunner, Error,
			   TEXT("Unable to find installed '%s' plugin in IPluginManager."), PluginName);
		return CharonIntermediateDirectory;
	}
	
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	const FString PluginScriptsDirectory = FPaths::ConvertRelativePathToFull(
		Plugin->GetBaseDir() / TEXT("Resources/Scripts"));

	if (!PlatformFile.DirectoryExists(*PluginScriptsDirectory))
	{
		UE_LOG(LogFCharonCliCommandRunner, Error,
			   TEXT("Missing resource directory for '%s' plugin at '%s'."), PluginName, *PluginScriptsDirectory);
		return CharonIntermediateDirectory;	
	}
	
	if (!PlatformFile.DirectoryExists(*CharonIntermediateDirectory))
	{
		if (!PlatformFile.CreateDirectory(*CharonIntermediateDirectory))
		{
			UE_LOG(LogFCharonCliCommandRunner, Error,
			       TEXT("Unable to create directory '%s' in Project's intermediate directory."), *CharonIntermediateDirectory);
			return CharonIntermediateDirectory;
		}
		bScriptsCopied = false;
	}

	if (!bScriptsCopied)
	{

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
			if (FPaths::GetExtension(TargetFileName) == TEXT("sh"))
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
		
		bScriptsCopied = true;
	}

	return CharonIntermediateDirectory;
}

FString FCharonCliCommandRunner::GetOrCreateCharonContentRootDirectory()
{
	const FString CharonIntermediateDirectory = FCharonCliCommandRunner::GetOrCreateCharonIntermediateDirectory();
	const FString PreferenceDirectory = CharonIntermediateDirectory / "preferences";
	IFileManager& FileManager = IFileManager::Get();
	if (!FileManager.DirectoryExists(*PreferenceDirectory))
	{
		FileManager.MakeDirectory(*PreferenceDirectory);
	}
	return PreferenceDirectory;
}
