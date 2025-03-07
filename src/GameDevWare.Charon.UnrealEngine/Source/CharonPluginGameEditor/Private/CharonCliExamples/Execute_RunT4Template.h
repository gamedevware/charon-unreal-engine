#pragma once
#include "GameData/CommandLine/FCharonCli.h"
#include "GameDataExtensionCommandsLog.h"
#include "UObject/Class.h"
#include "UObject/UObjectIterator.h"
#include "UObject/PropertyIterator.h"

static TSharedPtr<ICharonTask> CurrentRunT4DocRefTask;

/**
* Example running T4 template file and getting generated C++ code.
*/
static void Execute_RunT4Template(const TArray<UObject*> ContextSensitiveObjects)
{
	const FString TemplatePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir(), TEXT("Source/CharonPluginGameEditor/UGetEnglishNameBlueprintFunctionLibrary.tt"));
	const FString OutputFilePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir(), TEXT("Source/CharonPluginGameEditor/UGetEnglishNameBlueprintFunctionLibrary.h"));

	FString DocumentTypes;

	// Iterate over all UClass instances
	for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
	{
		UClass* CurrentClass = *ClassIt;

		// Check if the class inherits from UGameDataDocument
		if (CurrentClass->IsChildOf(UGameDataDocument::StaticClass()))
		{
			// Check if the class has a property named "NameRaw"
			if (CurrentClass->FindPropertyByName(TEXT("NameRaw")))
			{
				if (!DocumentTypes.IsEmpty())
				{
					DocumentTypes.Append(",");
				}
				DocumentTypes.Append(CurrentClass->GetPrefixCPP() + CurrentClass->GetName());
			}
		}
	}
	
	TArray<TPair<FString, FString>> Parameters = TArray<TPair<FString, FString>>();
	Parameters.Add(TPair<FString, FString>(TEXT("classesWithName"), DocumentTypes));
	
	const auto PreprocessTask = FCharonCli::RunT4
	(
		TemplatePath,
		TArray<FString>(), // ReferencedAssemblies
		TArray<FString>(), // Usings
		TArray<FString>(), // IncludeDirectories
		TArray<FString>(), // AssemblyLookupDirectories
		Parameters
	);

	PreprocessTask->OnCommandSucceed().AddLambda([](const FString& Output)
	{
		UE_LOG(LogFGameDataExtensionCommands, Log, TEXT("Successfully transformed T4 template into C++ code. Output:\n %s."), *Output);
	});
	
	PreprocessTask->OnCommandFailed().AddLambda([](const int ExitCode, const FString& Output)
	{
		UE_LOG(LogFGameDataExtensionCommands, Warning, TEXT("Command run failed with exit code %d. Output:\n %s."), ExitCode, *Output);
	});
	
	PreprocessTask->Start(/* EventDispatchThread */ ENamedThreads::GameThread);

	//
	// Make sure that Task will outlive this method call.
	//
	CurrentPreprocessT4Task = PreprocessTask;
}