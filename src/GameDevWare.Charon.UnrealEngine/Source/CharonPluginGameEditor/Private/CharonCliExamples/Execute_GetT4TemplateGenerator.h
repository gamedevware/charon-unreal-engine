#pragma once
#include "GameData/CommandLine/FCharonCli.h"
#include "GameDataExtensionCommandsLog.h"
#include "HAL/PlatformFilemanager.h"

static TSharedPtr<ICharonTask> CurrentPreprocessT4Task;

/**
* Example of preprocessing T4 template file into C# text generator code.
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
		UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Successfully preprocessed T4 into C# code. Output:\n %s."), *Output);

		//
		// Remove generated file, this is done in this example only to avoid creating garbage, you will probably want to save and use this file further.
		//
		FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*OutputFilePath);
	});
	
	PreprocessTask->OnCommandFailed().AddLambda([OutputFilePath](const int ExitCode, const FString& Output)
	{
		UE_LOG(LogFGameDataExtensionCommands, Warning, TEXT("Command run failed with exit code %d. Output:\n %s."), ExitCode, *Output);

		FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*OutputFilePath);
	});
	
	PreprocessTask->Start(/* EventDispatchThread */ ENamedThreads::GameThread);

	//
	// Make sure that Task will outlive this method call.
	//
	CurrentPreprocessT4Task = PreprocessTask;
}