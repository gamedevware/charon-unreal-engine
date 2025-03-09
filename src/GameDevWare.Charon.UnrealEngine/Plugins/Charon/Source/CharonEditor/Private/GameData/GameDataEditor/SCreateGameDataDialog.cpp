// Copyright GameDevWare, Denis Zykov 2025

#include "SCreateGameDataDialog.h"

#include "GameData/FDeferredGameDataImporter.h"
#include "GameProjectGenerationModule.h"
#include "IHotReload.h"
#include "ProjectDescriptor.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/Application/SlateApplication.h"
#include "SWarningOrErrorBox.h"
#include "UnrealEdGlobals.h"
#include "AutoReimport/AutoReimportManager.h"
#include "Dom/JsonObject.h"
#include "Editor/UnrealEdEngine.h"
#include "GameData/FRenamedCharonTask.h"
#include "GameData/ICharonEditorModule.h"
#include "GameData/ICharonTask.h"
#include "GameData/CommandLine/FCharonCli.h"
#include "Interfaces/IProjectManager.h"
#include "Misc/FileHelper.h"
#include "Widgets/Notifications/SProgressBar.h"

DEFINE_LOG_CATEGORY(LogSCreateGameDataDialog);

void SCreateGameDataDialog::Construct(const FArguments& InArgs)
{
	Name = FPaths::GetCleanFilename(InArgs._AssetPath);
	AssetPath = InArgs._AssetPath;
	Extension = TEXT(".gdjs");
	bRestartRequired = false;

	OnNameChanges(FText::FromString(Name));

	if (AssetPath.IsEmpty())
	{
		ErrorText = INVTEXT("Failed to determine asset path in file system. 'SCreateGameDataDialog.AssetPath' is not set.");
		return;
	}

	SWindow::Construct(SWindow::FArguments()
	                   .Title(InArgs._Title)
	                   .SizingRule(ESizingRule::Autosized)
	                   .AutoCenter(EAutoCenter::PreferredWorkArea)
	                   .IsTopmostWindow(true)
	                   .FocusWhenFirstShown(true)
	                   .SaneWindowPlacement(true)
	                   .SupportsMaximize(false)
	                   .SupportsMinimize(false)
	                   .MaxWidth(1600)
	                   .Content()
		[
			SNew(SBorder)
			.Padding(18)
			.BorderImage(FAppStyle::GetBrush("Docking.Tab.ContentAreaBrush"))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				[
					SAssignNew(MainWizard, SWizard)
					.ShowPageList(false)
					.FinishButtonText(this, &SCreateGameDataDialog::GetFinishText)
					.CanFinish(this, &SCreateGameDataDialog::CanFinish)
					.OnCanceled(this, &SCreateGameDataDialog::CancelClicked)
					.OnFinished(this, &SCreateGameDataDialog::FinishClicked)
					.InitialPageIndex(0)

					// Enter Game Data Name
					+ SWizard::Page()
					[
						SNew(SVerticalBox)

						// Title
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0)
						[
							SNew(STextBlock)
							.Font(FAppStyle::Get().GetFontStyle("HeadingExtraSmall"))
							.Text(INVTEXT("Enter Name For Game Data"))
							.TransformPolicy(ETextTransformPolicy::ToUpper)
						]

						// Name Input
						+ SVerticalBox::Slot()
						.Padding(0, 10, 0, 10)
						.AutoHeight()
						[
							SNew(SEditableTextBox)
							.HintText(INVTEXT("MyGameData"))
							.Text(FText::FromString(Name))
							.OnTextChanged(this, &SCreateGameDataDialog::OnNameChanges)
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)

							+ SHorizontalBox::Slot()
							.HAlign(HAlign_Left)
							[
								SNew(SHyperlink)
								.OnNavigate(this, &SCreateGameDataDialog::GoToDocumentation)
								.Text(INVTEXT("Guide: How to Create Game Data File."))
							]
						]

						// Name Error Label
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0, 5)
						[
							SNew(SWarningOrErrorBox)
							.MessageStyle(EMessageStyle::Error)
							.Visibility(this, &SCreateGameDataDialog::GetErrorLabelVisibility)
							.Message(this, &SCreateGameDataDialog::GetErrorLabelText)
						]
					]

					// Generating Data
					+ SWizard::Page()
					.CanShow(this, &SCreateGameDataDialog::CanGenerateCodeAndAssets)
					.OnEnter(this, &SCreateGameDataDialog::OnGenerateCodeAndAssets)
					[
						SNew(SVerticalBox)

						// Title
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0)
						[
							SNew(STextBlock)
							.Font(FAppStyle::Get().GetFontStyle("HeadingExtraSmall"))
							.Text(INVTEXT("Generating Code and Assets"))
							.TransformPolicy(ETextTransformPolicy::ToUpper)
						]

						// Progress Name
						+ SVerticalBox::Slot()
						.Padding(0, 10, 0, 10)
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(this, &SCreateGameDataDialog::GetGenerationProgressText)
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SProgressBar)
							.Percent(this, &SCreateGameDataDialog::GetGenerationProgress)
						]

						// Generation Error Label
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0, 5)
						[
							SNew(SWarningOrErrorBox)
							.MessageStyle(EMessageStyle::Error)
							.Visibility(this, &SCreateGameDataDialog::GetErrorLabelVisibility)
							.Message(this, &SCreateGameDataDialog::GetErrorLabelText)
						]
					]

					// Summary
					+ SWizard::Page()
					.CanShow(this, &SCreateGameDataDialog::CanFinish)
					[
						SNew(SVerticalBox)

						// Title
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0)
						[
							SNew(STextBlock)
							.Font(FAppStyle::Get().GetFontStyle("HeadingExtraSmall"))
							.Text(INVTEXT("Further steps"))
							.TransformPolicy(ETextTransformPolicy::ToUpper)
						]

						// Further steps 
						+ SVerticalBox::Slot()
						.Padding(0, 10, 0, 10)
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(this, &SCreateGameDataDialog::GetFurtherStepsText)
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)

							+ SHorizontalBox::Slot()
							.HAlign(HAlign_Left)
							[
								SNew(SHyperlink)
								.OnNavigate(this, &SCreateGameDataDialog::GoToDocumentation)
								.Text(INVTEXT("Guide: How to Create Game Data File."))
							]
						]

						// Generation Error Label
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0, 5)
						[
							SNew(SWarningOrErrorBox)
							.MessageStyle(EMessageStyle::Error)
							.Visibility(this, &SCreateGameDataDialog::GetErrorLabelVisibility)
							.Message(this, &SCreateGameDataDialog::GetErrorLabelText)
						]
					]
				]
			]
		]);
}

void SCreateGameDataDialog::ShowModal()
{
	FSlateApplication::Get().AddModalWindow(StaticCastSharedRef<SWindow>(this->AsShared()), FGlobalTabmanager::Get()->GetRootWindow());
}

void SCreateGameDataDialog::Show()
{
	const TSharedRef<SWindow> Window = FSlateApplication::Get().AddWindow(StaticCastSharedRef<SWindow>(this->AsShared()), true);
}

EVisibility SCreateGameDataDialog::GetErrorLabelVisibility() const
{
	return ErrorText.IsEmpty() ? EVisibility::Hidden : EVisibility::Visible;
}

void SCreateGameDataDialog::OnNameChanges(const FText& Text)
{
	Name.Empty();
	ErrorText = FText::GetEmpty();
	if (Text.IsEmpty())
	{
		return;
	}

	const FString FileName = Text.ToString();
	bool bFileNameIsValid = true;
	for (const auto FileNameChar : FileName)
	{
		if (!FChar::IsAlnum(FileNameChar))
		{
			bFileNameIsValid = false;
			break;
		}
	}
	if (FChar::IsDigit(FileName[0]) || !bFileNameIsValid)
	{
		ErrorText = FText::Format(INVTEXT("Name '{0}' must be composed solely of Latin letters (A-Z, a-z), cannot include spaces, and must not begin with a number."), FText::FromString(FileName));
		return;
	}


	const FName ModuleName = FName(FileName);
	if (FModuleManager::Get().GetModule(ModuleName) != nullptr)
	{
		ErrorText = FText::Format(INVTEXT("Invalid name '{0}'. A module with the same name already exists."), FText::FromString(ModuleName.ToString()));
		return;
	}

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	const FString ModuleDirectory = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("Source") / FileName);
	
	if (PlatformFile.DirectoryExists(*ModuleDirectory))
	{
		ErrorText = FText::Format(INVTEXT("Invalid name '{0}'. A module directory with the same name '{1}' already exists."), FText::FromString(FileName), FText::FromString(ModuleDirectory));
		return;
	}

	Name = FileName;
}

bool SCreateGameDataDialog::CanGenerateCodeAndAssets() const
{
	return !Name.IsEmpty() && !bIsFinished;
}

void SCreateGameDataDialog::OnGenerateCodeAndAssets()
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FString GameDataFilePath = FPaths::GetPath(AssetPath) / Name + Extension;
	FString ModuleDirectory = TEXT("Source") / Name;
	const FName ModuleName = FName(Name);

	ModuleDirectory = FPaths::ConvertRelativePathToFull(FPaths::ConvertRelativePathToFull(FPaths::ProjectDir(), ModuleDirectory));
	GameDataFilePath = FPaths::ConvertRelativePathToFull(FPaths::ConvertRelativePathToFull(FPaths::ProjectDir(), GameDataFilePath));

	GUnrealEd->AutoReimportManager->IgnoreNewFile(GameDataFilePath);
	GUnrealEd->AutoReimportManager->IgnoreFileModification(GameDataFilePath);
	
	const FString EmptyGameData;
	if (!PlatformFile.FileExists(*GameDataFilePath) && !FFileHelper::SaveStringToFile(EmptyGameData, *GameDataFilePath))
	{
		ErrorText = FText::Format(INVTEXT("Failed to create the game data file '{0}' due to a file system-related error. Make sure you have permission to write to the specified directory."), FText::FromString(GameDataFilePath));
		return;
	}

	TArray<TSharedRef<ICharonTask>> Tasks;
	if (PlatformFile.FileSize(*GameDataFilePath) == 0)
	{
		const FString EmptyApiKey;
		const TSharedRef<FJsonObject> ProjectSettingsDocument = MakeShared<FJsonObject>();
		ProjectSettingsDocument->SetStringField(TEXT("Name"), Name);
		const auto CreateNewGameDataClassTask = FCharonCli::UpdateDocument(
			GameDataFilePath,
			EmptyApiKey,
			TEXT("ProjectSettings"),
			ProjectSettingsDocument,
			TEXT("049bc0604c363a980b000088")
		);

		Tasks.Add(MakeShared<FRenamedCharonTask>(CreateNewGameDataClassTask, INVTEXT("Initializing Game Data File...")));
	}

	const auto GenerateCppCodeTask = FCharonCli::GenerateUnrealEngineSourceCode(
		GameDataFilePath,
		TEXT(""),
		ModuleDirectory,
		TEXT("UDocument"),
		TEXT("U") + Name
	);
	Tasks.Add(GenerateCppCodeTask);
	
	Tasks.Add(ICharonTask::FromSimpleDelegate(
		FSimpleDelegate::CreateSP(this, &SCreateGameDataDialog::AddModuleToProjectFile, ModuleName),
		INVTEXT("Adding module to .uproject and targets...")));

	Tasks.Add(ICharonTask::FromSimpleDelegate(
		FSimpleDelegate::CreateSP(this, &SCreateGameDataDialog::UpdateCodeProjectFiles),
		INVTEXT("Regenerating IDE project files...")));

	Tasks.Add(ICharonTask::FromSimpleDelegate(
		FSimpleDelegate::CreateSP(this, &SCreateGameDataDialog::CompileModule, ModuleName),
		FText::Format(INVTEXT("Compiling '{0}' module..."), FText::FromString(Name))));

	Tasks.Add(ICharonTask::FromSimpleDelegate(
		FSimpleDelegate::CreateSP(this, &SCreateGameDataDialog::TryImportGameData, ModuleName, GameDataFilePath),
		FText::Format(INVTEXT("Trying to import '{0}' ..."), FText::FromString(Name))));

	GenerateTask = ICharonTask::AsSequentialRunner(Tasks, ETaskFailureHandling::FailOnFirstError);

	const int32 TaskNum = Tasks.Num();
	if (Tasks.Num() > 0)
	{
		int32 TaskIndex = 0;
		for (const auto IntermediateTask : Tasks)
		{
			IntermediateTask->OnStart().AddSP(this, &SCreateGameDataDialog::ReportGenerationProgress, IntermediateTask, TaskIndex, TaskNum);
			TaskIndex++;
		}
	}

	GenerateTask->OnSucceed().AddSP(this, &SCreateGameDataDialog::CodeGenerationCompleted);
	GenerateTask->OnFailed().AddSP(this, &SCreateGameDataDialog::CodeGenerationFailed);

	GenerateTask->Start(ENamedThreads::GameThread);
}

void SCreateGameDataDialog::ReportGenerationProgress(TSharedRef<ICharonTask> Task, int TaskIndex, int TaskNum)
{
	GenerationProgressText = FText::Format(INVTEXT("Step {0}: {1}"), TaskIndex + 1, Task->GetDisplayName());
	GenerationProgress = (TaskIndex + 0.0f) / TaskNum;

	BringToFront(/*bForce*/ true);
}

void SCreateGameDataDialog::CodeGenerationCompleted()
{
	bIsFinished = true;
	MainWizard->AdvanceToPage(2); // Summary

	BringToFront(/*bForce*/ true);

	FString GameDataFilePath = FPaths::GetPath(AssetPath) / Name + Extension;
	FPaths::MakePathRelativeTo(GameDataFilePath, *FPaths::ProjectDir());

	FurtherStepsText = FText::Format(
		INVTEXT("The following steps are completed:\r\n\t ✅ Created game data file - {0}.\r\n\t ✅ Generated C++ code module - {1}.\r\n\t ✅ Added module to the current .uproject file.\r\n\t ✅ Added module to .Target.cs files.\r\n\t {2} Imported game data file into '.uasset'.\r\n\r\n{3}"),
		FText::FromString(GameDataFilePath),
		FText::FromString(Name),
		FText::FromString(bRestartRequired ? TEXT("❌") : TEXT("✅")),
		FText::FromString(bRestartRequired ? TEXT("Game data will be automatically imported after Unreal Editor restarts.") : TEXT(""))
	);
}

void SCreateGameDataDialog::CodeGenerationFailed()
{
	GenerationProgressText = FText::GetEmpty();
	GenerationProgress = 1.0;

	ErrorText = INVTEXT("The source code generation process encountered errors and did not complete successfully. Please consult the Output Log for specific error details, address these issues, and attempt the generation process again. Note that there may have been partial changes made to the project files; it's advisable to review these modifications in your Source Control system and consider discarding them if necessary.");
}

void SCreateGameDataDialog::AddModuleToProjectFile(const FName ModuleName) const
{
	IProjectManager& ProjectManager = IProjectManager::Get();
	auto CurrentProject = FProjectDescriptor(*ProjectManager.Get().GetCurrentProject());

	if (!CurrentProject.HasModule(ModuleName))
	{
		UE_LOG(LogSCreateGameDataDialog, Log, TEXT("Inserting module '%s' into the current .uproject file."), *ModuleName.ToString());

		CurrentProject.Modules.Add(FModuleDescriptor(ModuleName, EHostType::Runtime, ELoadingPhase::Default));
	}

	FText FailReason;
	if (!CurrentProject.Save(FPaths::GetProjectFilePath(), FailReason))
	{
		UE_LOG(LogSCreateGameDataDialog, Error, TEXT("Failed to add the module '%s' to the .uproject file due to an error - %s. Please check the Output Log, fix any errors, and try adding module and compiling code manually."), *ModuleName.ToString(), *FailReason.ToString());
	}

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	const FString TargetFileDirectory = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), TEXT("Source")));

	TArray<FString> FoundTargetFiles;
	PlatformFile.FindFiles(FoundTargetFiles, *TargetFileDirectory, TEXT(".cs"));
	for (FString TargetFilePath : FoundTargetFiles)
	{
		if (!TargetFilePath.EndsWith(".Target.cs"))
		{
			continue;
		}

		TArray<FString> FileLines;
		if (!FFileHelper::LoadFileToStringArray(FileLines, *TargetFilePath))
		{
			UE_LOG(LogSCreateGameDataDialog, Warning, TEXT("Failed to add the module '%s' to the '%s'. Please check the Output Log, fix any errors, and try adding module and compiling code manually."), *ModuleName.ToString(), *TargetFilePath);
			continue;
		}

		UE_LOG(LogSCreateGameDataDialog, Log, TEXT("Inserting module '%s' into the '%s' file."), *ModuleName.ToString(), *TargetFilePath);

		const FString QuotedModuleName = FString::Format(TEXT("\"{0}\""), {ModuleName.ToString()});
		if (!FileLines.ContainsByPredicate([QuotedModuleName](const FString& Content)
		{
			return Content.Contains(QuotedModuleName);
		}))
		{
			FString SearchPattern = TEXT("ExtraModuleNames.");
			const int32 InsertPosition = FileLines.IndexOfByPredicate([SearchPattern](const FString& Content)
			{
				return Content.Contains(SearchPattern);
			});
			if (InsertPosition != INDEX_NONE)
			{
				FString InsertString = FileLines[InsertPosition];
				const int32 CutPosition = InsertString.Find(SearchPattern) + SearchPattern.Len();
				InsertString.MidInline(0, CutPosition);
				InsertString.Appendf(TEXT("Add(%s);"), *QuotedModuleName);
				FileLines.Insert(InsertString, InsertPosition);
			}
		}

		if (!FFileHelper::SaveStringArrayToFile(FileLines, *TargetFilePath))
		{
			UE_LOG(LogSCreateGameDataDialog, Warning, TEXT("Failed to add the module '%s' to the '%s'. Please check the Output Log, fix any errors, and try adding module and compiling code manually."), *ModuleName.ToString(), *TargetFilePath);
			continue;
		}
	}
}

void SCreateGameDataDialog::UpdateCodeProjectFiles() const
{
	FText FailReason, FailLog;
	if (!FGameProjectGenerationModule::Get().UpdateCodeProject(FailReason, FailLog))
	{
		UE_LOG(LogSCreateGameDataDialog, Warning, TEXT("Failed to update to the IDE project files due to an error. %s - %s. Please check the Output Log, fix any errors, and try compiling code manually."), *FailReason.ToString(), *FailLog.ToString());
	}
}

void SCreateGameDataDialog::CompileModule(FName ModuleName) const
{
	IHotReloadModule& HotReloadModule = IHotReloadModule::Get();
	const bool bRecompileSucceeded = HotReloadModule.RecompileModule(ModuleName, *GLog, ERecompileModuleFlags::ReloadAfterRecompile);
	if (!bRecompileSucceeded)
	{
		UE_LOG(LogSCreateGameDataDialog, Error, TEXT("Failed to recompile the C++ code due error. Please check the Output Log, fix any errors, and try adding module and compiling code manually."));
		return;
	}
}

void SCreateGameDataDialog::TryImportGameData(FName ModuleName, FString GameDataFile)
{
	FPaths::MakePathRelativeTo(GameDataFile, *FPaths::ProjectDir());

	if (!FDeferredGameDataImporter::TryToImportGameData(GameDataFile, ModuleName.ToString(), ModuleName.ToString()))
	{
		bRestartRequired = true;

		UE_LOG(LogSCreateGameDataDialog, Warning, TEXT("Failed to auto-import game data file '%s'. Unreal Editor restart is required."), *GameDataFile);

		FDeferredGameDataImporter::RegisterGameDataToImport(GameDataFile, ModuleName.ToString(), ModuleName.ToString());
	}
}

FText SCreateGameDataDialog::GetFinishText() const
{
	return bRestartRequired ? INVTEXT("Restart Editor") : INVTEXT("Finish");
}

bool SCreateGameDataDialog::CanFinish() const
{
	return bIsFinished;
}

void SCreateGameDataDialog::GoToDocumentation()
{
	const FString DocumentationUrl = TEXT("https://gamedevware.github.io/charon/unreal_engine/creating_game_data.html");
	FString LaunchError;
	FPlatformProcess::LaunchURL(*DocumentationUrl, nullptr, &LaunchError);
	ErrorText = FText::FromString(LaunchError);
}

void SCreateGameDataDialog::CloseWindow()
{
	const auto ContainingWindow = FSlateApplication::Get().FindWidgetWindow(AsShared());
	if (ContainingWindow.IsValid())
	{
		ContainingWindow->RequestDestroyWindow();
	}
}

void SCreateGameDataDialog::CancelClicked()
{
	auto _ = this->OnFinished.ExecuteIfBound(false);
	CloseWindow();
}

void SCreateGameDataDialog::FinishClicked()
{
	auto _ = this->OnFinished.ExecuteIfBound(true);
	if (this->bRestartRequired)
	{
		const bool bWarn = false;
		FUnrealEdMisc::Get().RestartEditor(bWarn);;
	}
	else
	{
		CloseWindow();
	}
}
