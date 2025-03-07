// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "GameData/CommandLine/FCharonCli.h"
#include "GameDataExtensionCommandsLog.h"
#include "HAL/PlatformFilemanager.h"

static TSharedPtr<ICharonTask> CurrentPreprocessT4Task;

/**
 * Example of preprocessing a T4 template file into C# text generator code.
 */
static void Execute_GetT4TemplateGenerator(const TArray<UObject*> ContextSensitiveObjects)
{
	const FString TemplatePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir(), TEXT("Source/CharonPluginGameEditor/UGetEnglishNameBlueprintFunctionLibrary.tt"));
	const FString OutputFilePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir(), TEXT("Source/CharonPluginGameEditor/UGetEnglishNameBlueprintFunctionLibrary.cs"));
	
	const auto PreprocessTask = FCharonCli::PreprocessT4
	(
		TemplatePath,
		OutputFilePath,
		TEXT("GetEnglishNameBlueprintFunctions") // TemplateClassName
	);

	PreprocessTask->OnCommandSucceed().AddLambda([OutputFilePath](const FString& Output)
	{
		UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Successfully preprocessed T4 template into C# code. Output:\n%s"), *Output);

		//
		// Remove the generated file. This is done in this example only to avoid creating unnecessary files.
		// In a real scenario, you would likely want to save and use this file.
		//
		FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*OutputFilePath);
	});
	
	PreprocessTask->OnCommandFailed().AddLambda([OutputFilePath](const int ExitCode, const FString& Output)
	{
		UE_LOG(LogFGameDataExtensionCommands, Warning, TEXT("Command execution failed with exit code %d. Output:\n%s"), ExitCode, *Output);

		// Clean up the output file in case of failure.
		FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*OutputFilePath);
	});
	
	PreprocessTask->Start(/* EventDispatchThread */ ENamedThreads::GameThread);

	//
	// Ensure the task outlives this method call by storing it in a shared pointer.
	//
	CurrentPreprocessT4Task = PreprocessTask;
}