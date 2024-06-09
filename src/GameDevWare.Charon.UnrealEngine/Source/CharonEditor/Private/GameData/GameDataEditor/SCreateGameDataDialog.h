﻿// Copyright GameDevWare, Denis Zykov 2024

#pragma once

#include "Widgets/Workflow/SWizard.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSCreateGameDataDialog, Log, All);

DECLARE_DELEGATE_OneParam(SCreateGameDataDialogFinished, bool)

class ICharonTask;

class CHARONEDITOR_API SCreateGameDataDialog : public SWindow
{
private:
	TSharedPtr<SWizard> MainWizard;
	TSharedPtr<ICharonTask> GenerateTask;
	SCreateGameDataDialogFinished OnFinished;
	FString AssetPath;
	FString Name;
	FString Extension;
	FText FurtherStepsText;
	FText ErrorText;
	FText GenerationProgressText;
	float GenerationProgress = 0;
	bool bIsFinished = false;
	bool bRestartRequired = false;
	
public:
	SLATE_BEGIN_ARGS(SCreateGameDataDialog)	{}
		SLATE_ARGUMENT(FText, Title)
		SLATE_ARGUMENT(FString, AssetPath)
		SLATE_EVENT(SCreateGameDataDialogFinished, OnFinished)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void ShowModal();
	void Show();
	
	static void FixCppCode(const FString ModuleDirectory);

private:
	EVisibility GetErrorLabelVisibility() const;
	FText GetErrorLabelText() const { return ErrorText; }
	void OnNameChanges(const FText& Text);
	bool CanGenerateCodeAndAssets() const;
	void OnGenerateCodeAndAssets();
	FText GetGenerationProgressText() const { return GenerationProgressText; }
	TOptional<float> GetGenerationProgress() const { return GenerationProgress; }
	void ReportGenerationProgress(TSharedRef<ICharonTask> Task, int TaskIndex, int TaskNum);
	void CodeGenerationCompleted();
	void CodeGenerationFailed();
	FText GetFurtherStepsText() const { return FurtherStepsText; }

	void AddModuleToProjectFile(const FName ModuleName) const;
	void UpdateCodeProjectFiles() const;
	void CompileModule(FName ModuleName) const;
	void TryImportGameData(FName ModuleName, FString GameDataPath);

	FText GetFinishText() const;
	bool CanFinish() const;
	void GoToDocumentation();
	
	void CloseWindow();
	void CancelClicked();
	void FinishClicked();
};
