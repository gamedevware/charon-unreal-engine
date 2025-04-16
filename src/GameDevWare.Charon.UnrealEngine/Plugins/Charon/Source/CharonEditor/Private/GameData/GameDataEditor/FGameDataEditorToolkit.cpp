// Copyright GameDevWare, Denis Zykov 2025

#include "FGameDataEditorToolkit.h"
#include "GameData/ServerApi/FApiKeyStorage.h"
#include "GameData/CommandLine/FCharonCli.h"
#include "FGameDataEditorCommands.h"
#include "IWebBrowserWindow.h"
#include "GameData/ICharonEditorModule.h"
#include "SConnectGameDataDialog.h"
#include "SourceCodeNavigation.h"
#include "SWebBrowser.h"
#include "Dialogs/Dialogs.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Math/UnrealMathUtility.h"
#include "SSetApiKeyDialog.h"
#include "UGameDataEditorWebBrowserBridge.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "GameData/UGameDataImportData.h"
#include "GenericPlatform/GenericPlatformHttp.h"

DEFINE_LOG_CATEGORY(LogFGameDataEditorToolkit);

const FString LoadingHtml = TEXT(
	"<style>.loadingspinner {pointer-events: none;width: 2.5em;height: 2.5em;border: 0.4em solid transparent;border-color: #eee;border-top-color: #3E67EC;border-radius: 50%;animation: loadingspin 1s linear infinite;}@keyframes loadingspin {100% {transform: rotate(360deg) }}body {height: 100vh;margin: 0;padding: 0;display: flex;justify-content: center;align-items: center;}</style><div class=\"loadingspinner\"></div>");
const FString CancelledHtml = TEXT("<h1>Process Start Cancelled</h1><p>Review Unreal Engine logs for details.</p>");
const FString ProcessFailedHtml = TEXT("<h1>Process Start Failed</h1><p>Review Unreal Engine logs for details.</p>");
const FString StartFailedHtml = TEXT("<h1>Start Failed</h1><p>Review Unreal Engine logs for details.</p>");


void FGameDataEditorToolkit::InitEditor(const TArray<UObject*>& InObjects)
{
	GameData = Cast<UGameDataBase>(InObjects[0]);

	WebBrowserBridge = NewObject<UGameDataEditorWebBrowserBridge>();
	WebBrowserBridge->EditorToolkit = StaticCastSharedRef<FGameDataEditorToolkit>(this->AsShared());
	
	BindCommands();

	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("GameDataEditorLayout")->AddArea
	(
		FTabManager::NewPrimaryArea()
		->SetOrientation(Orient_Vertical)
		->Split
		(
			FTabManager::NewSplitter()
			->SetSizeCoefficient(0.6f)
			->SetOrientation(Orient_Horizontal)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.8f)
				->AddTab("GameDataBrowserTab", ETabState::OpenedTab)
			)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.2f)
				->AddTab("GameDataDetailsTab", ETabState::OpenedTab)
			)
		)
	);

	InitAssetEditor(EToolkitMode::Standalone, {}, TEXT("GameDataEditor"), Layout, true, true, InObjects);

	if (!Browser)
	{
		InvokeTab(FName("GameDataBrowserTab"));
	}

	Browser->BindUObject("deployment", WebBrowserBridge, /* permanent */ true);
	
	ExtendToolbar();
	ExtendMenu();

	RegenerateMenusAndToolbars();

	if (CanDisconnect())
	{
		OpenCharonWebsite();
	}
	else
	{
		LaunchCharonProcess();
	}
}

void FGameDataEditorToolkit::BindCommands()
{
	const TSharedRef<FUICommandList>& UICommandList = GetToolkitCommands();

	UICommandList->MapAction(
		FGameDataEditorCommands::Get().BrowserBack,
		FExecuteAction::CreateLambda([this] { if (Browser != nullptr) Browser->GoBack(); }),
		FCanExecuteAction::CreateLambda([this] { return Browser != nullptr && Browser->CanGoBack(); }));

	UICommandList->MapAction(
		FGameDataEditorCommands::Get().BrowserForward,
		FExecuteAction::CreateLambda([this] { if (Browser != nullptr) Browser->GoForward(); }),
		FCanExecuteAction::CreateLambda([this] { return Browser != nullptr && Browser->CanGoForward(); }));

	UICommandList->MapAction(
		FGameDataEditorCommands::Get().BrowserReload,
		FExecuteAction::CreateLambda([this] { if (Browser != nullptr) Browser->Reload(); }),
		FCanExecuteAction::CreateLambda([this] { return Browser != nullptr; }));

	UICommandList->MapAction(
		FGameDataEditorCommands::Get().BrowserOpenExternal,
		FExecuteAction::CreateSP(this, &FGameDataEditorToolkit::OpenInBrowser_Execute),
		FCanExecuteAction::CreateLambda([this] { return Browser != nullptr; }));
	
	UICommandList->MapAction(
		FGameDataEditorCommands::Get().Reimport,
		FExecuteAction::CreateSP(this, &FGameDataEditorToolkit::Sync_Execute),
		FCanExecuteAction::CreateSP(this, &FGameDataEditorToolkit::CanReimport));

	UICommandList->MapAction(
		FGameDataEditorCommands::Get().GenerateSourceCode,
		FExecuteAction::CreateSP(this, &FGameDataEditorToolkit::GenerateSourceCode_Execute),
		FCanExecuteAction::CreateSP(this, &FGameDataEditorToolkit::CanGenerateSourceCode));

	UICommandList->MapAction(
		FGameDataEditorCommands::Get().Connect,
		FExecuteAction::CreateSP(this, &FGameDataEditorToolkit::Connect_Execute),
		FCanExecuteAction::CreateSP(this, &FGameDataEditorToolkit::CanConnect));

	UICommandList->MapAction(
		FGameDataEditorCommands::Get().Disconnect,
		FExecuteAction::CreateSP(this, &FGameDataEditorToolkit::Disconnect_Execute),
		FCanExecuteAction::CreateSP(this, &FGameDataEditorToolkit::CanDisconnect));

	UICommandList->MapAction(
		FGameDataEditorCommands::Get().Synchronize,
		FExecuteAction::CreateSP(this, &FGameDataEditorToolkit::Sync_Execute),
		FCanExecuteAction::CreateSP(this, &FGameDataEditorToolkit::CanDisconnect));

	UICommandList->MapAction(
		FGameDataEditorCommands::Get().SetApiKey,
		FExecuteAction::CreateSP(this, &FGameDataEditorToolkit::SetApiKey_Execute),
		FCanExecuteAction::CreateSP(this, &FGameDataEditorToolkit::CanSetApiKey));
}

void FGameDataEditorToolkit::ExtendToolbar()
{
	const TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);

	ToolbarExtender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateLambda([](FToolBarBuilder& ToolbarBuilder)
		{
			ToolbarBuilder.BeginSection(TEXT("Browser"));
			{
				ToolbarBuilder.AddToolBarButton(FGameDataEditorCommands::Get().BrowserBack,
				                                NAME_None, FText(), TAttribute<FText>(),
				                                FSlateIcon(FAppStyle::GetAppStyleSetName(),
				                                           "GraphBreadcrumb.BrowseBack")
				);
				ToolbarBuilder.AddToolBarButton(FGameDataEditorCommands::Get().BrowserForward,
				                                NAME_None, FText(), TAttribute<FText>(),
				                                FSlateIcon(FAppStyle::GetAppStyleSetName(),
				                                           "GraphBreadcrumb.BrowseForward")
				);
				ToolbarBuilder.AddToolBarButton(
					FGameDataEditorCommands::Get().BrowserReload,
					NAME_None, FText(), TAttribute<FText>(),
					FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Refresh")
				);
				ToolbarBuilder.AddToolBarButton(
					FGameDataEditorCommands::Get().BrowserOpenExternal,
					NAME_None, FText(), TAttribute<FText>(),
					FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Link")
				);
				ToolbarBuilder.AddSeparator();

				ToolbarBuilder.AddToolBarButton(FGameDataEditorCommands::Get().Reimport,
				                                NAME_None, FText(), TAttribute<FText>(),
				                                FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Import"));

				ToolbarBuilder.AddToolBarButton(FGameDataEditorCommands::Get().GenerateSourceCode,
				                                NAME_None, FText(), TAttribute<FText>(),
				                                FSlateIcon(FAppStyle::GetAppStyleSetName(), "MainFrame.AddCodeToProject"));
			}
			ToolbarBuilder.EndSection();
		})
	);

	AddToolbarExtender(ToolbarExtender);

	ICharonEditorModule* CharonEditorModule = &FModuleManager::LoadModuleChecked<ICharonEditorModule>(
		"CharonEditor");
	AddToolbarExtender(CharonEditorModule->GetGameDataEditorToolBarExtensibilityManager()->GetAllExtenders());
}

void FGameDataEditorToolkit::ExtendMenu()
{
	const TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender);

	MenuExtender->AddMenuExtension(
		"AssetEditorActions",
		EExtensionHook::After,
		GetToolkitCommands(),
		FMenuExtensionDelegate::CreateLambda([](FMenuBuilder& MenuBuilder)
		{
			MenuBuilder.BeginSection("SyncControl", INVTEXT("Sync Control"));
			MenuBuilder.AddMenuEntry(FGameDataEditorCommands::Get().Connect,
			                         NAME_None, TAttribute<FText>(), TAttribute<FText>(),
			                         FSlateIcon(FAppStyle::GetAppStyleSetName(), "Plus"));
			MenuBuilder.AddMenuEntry(FGameDataEditorCommands::Get().Disconnect,
			                         NAME_None, TAttribute<FText>(), TAttribute<FText>(),
			                         FSlateIcon(FAppStyle::GetAppStyleSetName(), "Cross"));
			MenuBuilder.AddMenuEntry(FGameDataEditorCommands::Get().Synchronize,
			                         NAME_None, TAttribute<FText>(), TAttribute<FText>(),
			                         FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Import"));
			MenuBuilder.AddMenuEntry(FGameDataEditorCommands::Get().SetApiKey,
			                         NAME_None, TAttribute<FText>(), TAttribute<FText>(),
			                         FSlateIcon(FAppStyle::GetAppStyleSetName(), "ProjectSettings.TabIcon"));
			MenuBuilder.EndSection();
		})
	);

	AddMenuExtender(MenuExtender);

	ICharonEditorModule* CharonEditorModule = &FModuleManager::LoadModuleChecked<ICharonEditorModule>(
		"CharonEditor");
	AddMenuExtender(CharonEditorModule->GetGameDataEditorMenuExtensibilityManager()->GetAllExtenders(GetToolkitCommands(), GetEditingObjects()));
}

void FGameDataEditorToolkit::OpenInBrowser_Execute() const
{
	const auto CurrentUrl = Browser->GetUrl(); 
	if(!Browser || !CurrentUrl.StartsWith("http"))
	{
		return;
	}

	FString Error;
	FPlatformProcess::LaunchURL(*CurrentUrl, nullptr, &Error);
	if (!Error.IsEmpty())
	{
		UE_LOG(LogSConnectGameDataDialog, Error, TEXT("Failed to open the URL '%s' in the OS browser due to an error. %s"), *CurrentUrl, *Error);
	}
}

void FGameDataEditorToolkit::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(INVTEXT("Game Data Editor"));

	InTabManager->RegisterTabSpawner("GameDataBrowserTab", FOnSpawnTab::CreateLambda([this](const FSpawnTabArgs&)
	            {
		            return SNew(SDockTab)
					.CanEverClose(false)
		            [
			            SAssignNew(Browser, SWebBrowser)
						.ShowErrorMessage(false)
						.ShowControls(false)
						.BrowserFrameRate(60)
						.SupportsTransparency(false)
						.BackgroundColor(FColor::White)
			            .PopupMenuMethod(EPopupMethod::UseCurrentWindow)
						.InitialURL(TEXT("about:blank"))
			            .OnConsoleMessage_Static( &FGameDataEditorToolkit::OnBrowserConsoleMessage)
		            ];
	            }))
	            .SetDisplayName(INVTEXT("Game Data"))
	            .SetGroup(WorkspaceMenuCategory.ToSharedRef());

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(
		"PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	const TSharedRef<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsView->SetObjects(TArray<UObject*>{GameData});
	InTabManager->RegisterTabSpawner("GameDataDetailsTab", FOnSpawnTab::CreateLambda([this, DetailsView](const FSpawnTabArgs&)
	            {
		            return SNew(SDockTab)
		            [
			            DetailsView
		            ];
	            }))
	            .SetDisplayName(INVTEXT("Details"))
	            .SetGroup(WorkspaceMenuCategory.ToSharedRef());
}

void FGameDataEditorToolkit::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
	InTabManager->UnregisterTabSpawner("GameDataBrowserTab");
	InTabManager->UnregisterTabSpawner("GameDataDetailsTab");
}

void FGameDataEditorToolkit::LaunchCharonProcess()
{
	if (!Browser) { return; } // tab is closed
	
	FString GameDataFilePath;
	if (CanReimport())
	{
		GameDataFilePath = GameData->AssetImportData->GetNormalizedGameDataPath();
	}
	
	if (GameDataFilePath.IsEmpty() || !FPaths::FileExists(GameDataFilePath))
	{
		if (GameDataFilePath.IsEmpty())
		{
			UE_LOG(LogFGameDataEditorToolkit, Error, TEXT("There is source file is set for asset '%s'."),
			       *GameData->GetPathName());
		}
		else
		{
			UE_LOG(LogFGameDataEditorToolkit, Error, TEXT("Unable to find source file '%s' for asset '%s'."),
			       *GameDataFilePath, *GameData->GetPathName());
		}
		Browser->LoadString(ProcessFailedHtml, TEXT("about:blank"));
		return;
	}

	const int32 Port = FMath::RandRange(10001, 65500);
	EditorProcess = MakeShared<FCharonEditorProcessRunner>(GameDataFilePath, Port, FTimespan::FromSeconds(30));
	EditorProcess->OnLaunched().BindLambda([this](EGameDataEditorLaunchStatus Status)
	{
		switch (Status)
		{
		case EGameDataEditorLaunchStatus::Succeed: Browser->LoadURL(EditorProcess->StartUrl);
			UE_LOG(LogFGameDataEditorToolkit, Log, TEXT("Loading address '%s' in embedded WebBrowser."), *EditorProcess->StartUrl);
			break;
		case EGameDataEditorLaunchStatus::Cancelled: Browser->LoadString(CancelledHtml, EditorProcess->StartUrl);
			break;
		case EGameDataEditorLaunchStatus::MissingRunScript:
		case EGameDataEditorLaunchStatus::Timeout:
		case EGameDataEditorLaunchStatus::Failed: Browser->LoadString(ProcessFailedHtml, EditorProcess->StartUrl);
			break;
		}
	});

	Browser->LoadString(LoadingHtml, EditorProcess->StartUrl);
	EditorProcess->Launch();
}

void FGameDataEditorToolkit::OpenCharonWebsite() const
{
	if (!CanDisconnect()) { return; }
	if (!Browser) { return; } // tab is closed

	const auto ProjectId = GameData->AssetImportData->ProjectId;
	const auto ProjectName = GameData->AssetImportData->ProjectName;
	const auto BranchId = GameData->AssetImportData->BranchId;
	const auto ServerAddress = GameData->AssetImportData->ServerAddress;
	auto ServerApiClient = FServerApiClient(ServerAddress);

	FString BranchAddress = ServerApiClient.GetGameDataUrl(ProjectId, BranchId);

	if (BranchAddress.IsEmpty())
	{
		Browser->LoadString(StartFailedHtml, TEXT("about:blank"));
		return;
	}

	FString ApiKey;
	if (!FApiKeyStorage::LoadApiKey(ServerAddress, ProjectId, ApiKey) ||
		!ServerApiClient.GetLoginCode(ApiKey, OnGetLoginCodeResponse::CreateSP(
			this, &FGameDataEditorToolkit::OnGetLoginCodeResponse, BranchAddress)))
	{
		BroadcastMissingApiKey(FText::FromString(ProjectName));
		Browser->LoadURL(BranchAddress);
		UE_LOG(LogFGameDataEditorToolkit, Log, TEXT("Loading address '%s' in embedded WebBrowser."), *BranchAddress);
	}
}

bool FGameDataEditorToolkit::CanGenerateSourceCode() const
{
	return this->CanReimport(); // if it has source file, then we can generate source code from it
}

void FGameDataEditorToolkit::GenerateSourceCode_Execute()
{
	if (!CanReimport(GameData))
	{
		UE_LOG(LogFGameDataEditorToolkit, Warning, TEXT("Unable to generate C++ Source code for game data file '%s' because it doesn't contains path to a source file."), *GameData->GetName());
		return;
	}
	FString GameDataClassPath;
	if (!FSourceCodeNavigation::FindClassHeaderPath(GameData->GetClass(), GameDataClassPath))
	{
		UE_LOG(LogFGameDataEditorToolkit, Warning, TEXT("Unable to generate C++ Source code for game data file '%s' because 'FSourceCodeNavigation' can't find source code location."), *GameData->GetName());
		return;
	}

	FString SourceCodePath = FPaths::ConvertRelativePathToFull(GameDataClassPath / "../../");
	FPaths::NormalizeDirectoryName(SourceCodePath);
	FPaths::CollapseRelativeDirectories(SourceCodePath);
	
	FString GameDataUrl = GameData->AssetImportData->GetNormalizedGameDataPath();
	FString ApiKey;
	if (GameData->AssetImportData->IsConnected())
	{
		const auto ProjectId = GameData->AssetImportData->ProjectId;
		const auto ProjectName = GameData->AssetImportData->ProjectName;
		const auto ServerAddress = GameData->AssetImportData->ServerAddress;

		if (!FApiKeyStorage::LoadApiKey(ServerAddress, ProjectId, ApiKey))
		{
			BroadcastMissingApiKey(FText::FromString(ProjectName));
			return;
		}

		const auto ServerApiClient = FServerApiClient(ServerAddress);
		GameDataUrl = ServerApiClient.GetGameDataUrl(ProjectId, GameData->AssetImportData->BranchId);
	}

	FString GameDataClassName = GameData->AssetImportData->GameDataClassName;
	FString GameDataDocumentClassName = GameData->AssetImportData->GameDataDocumentClassName;
	const FString DefineConstants = GameData->AssetImportData->DefineConstants;
	const int SourceCodeOptimizationFlags = GameData->AssetImportData->Optimizations;
	const int SourceCodeLineEnding = GameData->AssetImportData->LineEnding;
	const int SourceCodeIndentation = GameData->AssetImportData->Indentation;

	if (GameDataClassName.IsEmpty())
	{
		GameDataClassName = GameData->GetClass()->GetPrefixCPP() + GameData->GetClass()->GetName();
	}
	if (GameDataDocumentClassName.IsEmpty())
	{
		GameDataDocumentClassName = "UDocument";
	}

	TArray<ESourceCodeGenerationOptimizations> SourceCodeOptimizations;
	for (int OptimizationValue = 0; OptimizationValue < 32; ++OptimizationValue)
	{
		if ((SourceCodeOptimizationFlags & (1 << OptimizationValue)) == 0) continue;

		SourceCodeOptimizations.Add(static_cast<ESourceCodeGenerationOptimizations>(OptimizationValue));	
	}
	
	const auto GenerateSourceCodeCommand = FCharonCli::GenerateUnrealEngineSourceCode(
		GameDataUrl, ApiKey, SourceCodePath, GameDataDocumentClassName, GameDataClassName, DefineConstants,
		SourceCodeOptimizations, static_cast<ESourceCodeIndentation>(SourceCodeIndentation), static_cast<ESourceCodeLineEndings>(SourceCodeLineEnding));

	BroadcastCommandRunning(
		GenerateSourceCodeCommand,
		 FSlateIcon(GetPluginStyleSet()->GetStyleSetName(), "Cpp128"),
		INVTEXT("Generating C++ source code..."),
		INVTEXT("C++ source code has been generated."),
		INVTEXT("C++ source code generation failed."),
		/* can cancel */ true
	);


	ICharonEditorModule& CharonEditorModule = ICharonEditorModule::Get(); 
	TArray<TSharedRef<ICharonTask>> AllTasks;
	auto PreTasks = MakeShared<TArray<TSharedRef<ICharonTask>>>();
	auto PostTasks = MakeShared<TArray<TSharedRef<ICharonTask>>>();
	
	CharonEditorModule.OnGameDataPreSourceCodeGeneration().Broadcast(GameData, PreTasks);
	CharonEditorModule.OnGameDataPostSourceCodeGeneration().Broadcast(GameData, PostTasks);

	AllTasks.Append(PreTasks.Get());
	AllTasks.Add(GenerateSourceCodeCommand);
	AllTasks.Append(PostTasks.Get());

	CurrentRunningCommand = ICharonTask::AsSequentialRunner(AllTasks);
	CurrentRunningCommand->Start(/* event dispatch thread */ ENamedThreads::GameThread);
}

bool FGameDataEditorToolkit::CanConnect() const
{
	return CanReimport() && !GameData->AssetImportData->IsConnected();
}

void FGameDataEditorToolkit::Connect_Execute()
{
	if (!CanConnect()) { return; }

	if (PendingDialog.IsValid())
	{
		PendingDialog->RequestDestroyWindow();
	}

	const auto ConnectGameDataDialog = SNew(SConnectGameDataDialog)
		.LocalGameDataFile(GameData->AssetImportData->GetNormalizedGameDataPath())
		.OnFinished(this, &FGameDataEditorToolkit::OnConnectFinished);
	ConnectGameDataDialog->Show();
	PendingDialog = ConnectGameDataDialog;
}

void FGameDataEditorToolkit::OnConnectFinished(FConnectGameDataParameters Parameters)
{
	PendingDialog = nullptr;
	if (!CanConnect()) { return; }

	GameData->AssetImportData->ProjectId = Parameters.Project->Id;
	GameData->AssetImportData->ProjectName = Parameters.Project->Name;
	GameData->AssetImportData->BranchId = Parameters.Branch->Id;
	GameData->AssetImportData->BranchName = Parameters.Branch->Name;
	GameData->AssetImportData->ServerAddress = Parameters.ServerAddress;

	auto _ = GameData->MarkPackageDirty();

	FApiKeyStorage::SaveApiKey(Parameters.ServerAddress, Parameters.Project->Id, Parameters.ApiKey);

	const auto ServerApiClient = FServerApiClient(Parameters.ServerAddress);
	const auto BranchAddress = ServerApiClient.GetGameDataUrl(Parameters.Project->Id, Parameters.Branch->Id);

	if (Parameters.InitialSyncDirection > 0)
	{
		const auto GameDataFilePath = GameData->AssetImportData->GetNormalizedGameDataPath();
		const auto Command = FCharonCli::RestoreFromFile(BranchAddress, Parameters.ApiKey, GameDataFilePath, "auto");

		BroadcastCommandRunning(
			Command,
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Import"),
			INVTEXT("Uploading game data ..."),
			INVTEXT("Game data has been uploaded."),
			INVTEXT("Game data upload failed."),
			/* can cancel */ false
		);

		Command->OnSucceed().AddSP(this, &FGameDataEditorToolkit::Sync_Execute);
		Command->OnSucceed().AddSP(this, &FGameDataEditorToolkit::OpenCharonWebsite);
		
		Command->Start(/* event dispatch thread */ ENamedThreads::GameThread);

		CurrentRunningCommand = Command; // prevent destruction of this command
	}
	else
	{
		Sync_Execute();
		OpenCharonWebsite();
	}
}

void FGameDataEditorToolkit::ReplaceGameDataFile(FString GameDataPath, FString ReplacementGameDataPath) const
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (!PlatformFile.FileExists(*GameDataPath) ||
		!PlatformFile.FileExists(*ReplacementGameDataPath))
	{
		return;
	}
	
	UE_LOG(LogFGameDataEditorToolkit, Log, TEXT("Replacing game data file '%s' with new game data file at '%s'."),
	       *GameDataPath,
	       *FPaths::GetCleanFilename(ReplacementGameDataPath)
	);

	PlatformFile.DeleteFile(*GameDataPath);
	PlatformFile.MoveFile(*GameDataPath, *ReplacementGameDataPath);
	PlatformFile.SetTimeStamp(*GameDataPath, FDateTime::Now());
}

void FGameDataEditorToolkit::OnGetLoginCodeResponse(FAuthenticationFlowStageResponse AuthenticationFlowStageResponse, FString AddressToOpen) const
{
	if (AuthenticationFlowStageResponse.Errors.Num() > 0)
	{
		const auto [Message, Code] = AuthenticationFlowStageResponse.Errors[0];
		UE_LOG(LogFGameDataEditorToolkit, Error, TEXT("Failed to authenticate with API Key due to an error: [%s] %s."), *Code, *Message);
	}
	
	const auto AuthorizationCode = AuthenticationFlowStageResponse.Result.AuthorizationCode;
	if (!AuthorizationCode.IsEmpty())
	{
		const auto OriginalPath = FGenericPlatformHttp::GetUrlPath(AddressToOpen, /* bIncludeQueryString */ true, /* bIncludeFragment */ false);
		const auto LoginParameters = FString::Format(TEXT("?loginCode={0}&returnUrl={1}"), {
			FGenericPlatformHttp::UrlEncode(AuthorizationCode),
			FGenericPlatformHttp::UrlEncode(OriginalPath)
		});

		const auto SignInPath = TEXT("/view/sign-in") + LoginParameters;
		const auto BaseDomainAndPort = FGenericPlatformHttp::GetUrlDomainAndPort(AddressToOpen);
		AddressToOpen = AddressToOpen.Mid(0, AddressToOpen.Find(BaseDomainAndPort) + BaseDomainAndPort.Len()) + SignInPath;
	}

	if (!Browser) { return; } // tab is closed
	
	Browser->LoadURL(AddressToOpen);
	UE_LOG(LogFGameDataEditorToolkit, Log, TEXT("Loading address '%s' in embedded WebBrowser."), *AddressToOpen);
}

bool FGameDataEditorToolkit::CanDisconnect() const
{
	return CanReimport() && GameData->AssetImportData->IsConnected();
}

void FGameDataEditorToolkit::SetApiKey_Execute()
{
	if (!CanDisconnect()) { return; }

	if (PendingDialog.IsValid())
	{
		PendingDialog->RequestDestroyWindow();
	}

	const auto SetApiKeyDialog = SNew(SSetApiKeyDialog)
		.ProjectName(GameData->AssetImportData->ProjectName)
		.ProjectId(GameData->AssetImportData->ProjectId)
		.ServerAddress(GameData->AssetImportData->ServerAddress)
		.OnFinished(this, &FGameDataEditorToolkit::OnSetApiKeyFinished);
	SetApiKeyDialog->Show();
	PendingDialog = SetApiKeyDialog;
}

bool FGameDataEditorToolkit::CanSetApiKey() const
{
	return CanDisconnect() && !FApiKeyStorage::IsKeyExists(GameData->AssetImportData->ServerAddress,
	                                                       GameData->AssetImportData->ProjectId);
}

void FGameDataEditorToolkit::OnSetApiKeyFinished(FString ApiKey) const
{
	if (!CanDisconnect()) { return; }

	const auto ProjectId = GameData->AssetImportData->ProjectId;
	const auto ServerAddress = GameData->AssetImportData->ServerAddress;

	FApiKeyStorage::SaveApiKey(ServerAddress, ProjectId, ApiKey);
}

void FGameDataEditorToolkit::Disconnect_Execute()
{
	const FText Message = INVTEXT("Do you want to disonnect from online project. Do you want to proceed?");
	const FText Title = INVTEXT("Disconnect confirmation");

	FSuppressableWarningDialog::FSetupInfo Info(Message, Title, "DisconnectGameData");
	Info.ConfirmText = INVTEXT("Yes");
	Info.CancelText = INVTEXT("No");
	const FSuppressableWarningDialog OpenDisconnectWarning(Info);
	if (OpenDisconnectWarning.ShowModal() == FSuppressableWarningDialog::Cancel)
	{
		return;
	}
	GameData->AssetImportData->Disconnect();
	auto _ = GameData->MarkPackageDirty();

	LaunchCharonProcess();
}

void FGameDataEditorToolkit::Sync_Execute()
{
	if (!CanReimport()) { return; }

	const FString GameDataPath = GameData->AssetImportData->GetNormalizedGameDataPath();
	const FString GameDataDownloadPath = GameDataPath + ".tmp";
	const TArray<FString> All { TEXT("*") };
	TSharedPtr<ICharonTask> BackupCommand;
	
	if (GameData->AssetImportData->IsConnected())
	{
		const auto ProjectId = GameData->AssetImportData->ProjectId;
		const auto ProjectName = GameData->AssetImportData->ProjectName;
		const auto ServerAddress = GameData->AssetImportData->ServerAddress;

		FString ApiKey;
		if (!FApiKeyStorage::LoadApiKey(ServerAddress, ProjectId, ApiKey))
		{
			BroadcastMissingApiKey(FText::FromString(ProjectName));
			return;
		}

		const auto ServerApiClient = FServerApiClient(ServerAddress);
		const FString GameDataUrl = ServerApiClient.GetGameDataUrl(ProjectId, GameData->AssetImportData->BranchId);


		BackupCommand = FCharonCli::BackupToFile(GameDataUrl, ApiKey, GameDataDownloadPath, FPaths::GetExtension(GameDataPath));
		
		BroadcastCommandRunning(
			BackupCommand.ToSharedRef(),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Import"),
			INVTEXT("Downloading game data..."),
			INVTEXT("Game data has been downloaded."),
			INVTEXT("Game data download failed."),
			/* can cancel */ true
		);

		BackupCommand->OnFailed().AddLambda([GameDataDownloadPath]()
		{
			// Clear temp file
			IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
			PlatformFile.DeleteFile(*GameDataDownloadPath);
		});
	}
	
	ICharonEditorModule& CharonEditorModule = ICharonEditorModule::Get(); 
	TArray<TSharedRef<ICharonTask>> AllTasks;
	auto PreTasks = MakeShared<TArray<TSharedRef<ICharonTask>>>();
	auto PostTasks = MakeShared<TArray<TSharedRef<ICharonTask>>>();
	CharonEditorModule.OnGameDataPreSynchronization().Broadcast(GameData, PreTasks);
	CharonEditorModule.OnGameDataPostSynchronization().Broadcast(GameData, PostTasks);

	AllTasks.Append(PreTasks.Get());
	if (BackupCommand != nullptr)
	{
		AllTasks.Add(BackupCommand.ToSharedRef());
		AllTasks.Add(ICharonTask::FromSimpleDelegate(FSimpleDelegate::CreateSP(this, &FGameDataEditorToolkit::ReplaceGameDataFile, GameDataPath, GameDataDownloadPath), INVTEXT("Replace Files")));
	}
	AllTasks.Add(ICharonTask::FromSimpleDelegate(FSimpleDelegate::CreateSP(this, &FGameDataEditorToolkit::Reimport_Execute), INVTEXT("Reimport")));
	AllTasks.Append(PostTasks.Get());

	CurrentRunningCommand = ICharonTask::AsSequentialRunner(AllTasks);
	CurrentRunningCommand->Start(/* event dispatch thread */ ENamedThreads::GameThread);
}

void FGameDataEditorToolkit::BroadcastCommandRunning(
	const TSharedRef<ICharonTask>& Command,
	FSlateIcon Icon,
	FText CommandPendingText,
	FText CommandSucceedText,
	FText CommandFailedText,
	bool bCanCancel)
{
	FNotificationInfo Info(CommandPendingText);
	Info.Image = Icon.GetIcon();
	if (bCanCancel)
	{
		FSimpleDelegate CancelCommandAction;
		const auto WeakCommand = TWeakPtr<ICharonTask>(Command);
		CancelCommandAction.BindLambda([WeakCommand]
		{
			const auto CommandPtr = WeakCommand.Pin();
			if (CommandPtr.IsValid()) CommandPtr->Stop();
		});
		Info.ButtonDetails.Add(FNotificationButtonInfo(
				INVTEXT("Cancel"),
				INVTEXT(""),
				CancelCommandAction)
		);
	}
	Info.FadeOutDuration = 3;
	Info.bFireAndForget = false;
	Info.bUseLargeFont = false;
	Info.bUseThrobber = false;
	Info.bUseSuccessFailIcons = true;

	const auto NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
	if (!NotificationItem.IsValid()) { return; }

	NotificationItem->SetCompletionState(SNotificationItem::CS_Pending);
	Command->OnSucceed().AddLambda([NotificationItem, CommandSucceedText]
	{
		NotificationItem->Pulse(FLinearColor::Green);
		NotificationItem->SetText(CommandSucceedText);
		NotificationItem->SetCompletionState(SNotificationItem::CS_Success);
		NotificationItem->Fadeout();
	});
	Command->OnFailed().AddLambda([NotificationItem, CommandFailedText]
	{
		NotificationItem->Pulse(FLinearColor::Red);
		NotificationItem->SetText(CommandFailedText);
		NotificationItem->SetCompletionState(SNotificationItem::CS_Fail);
		NotificationItem->Fadeout();
	});
}

void FGameDataEditorToolkit::BroadcastMissingApiKey(FText ProjectName)
{
	const FText ErrorText = FText::Format(
		INVTEXT("Missing saved API Key for '{0}' project. Go to 'Menu -> Assets -> Set API Key' setup one."),
		ProjectName);
	FNotificationInfo Info(ErrorText);
	Info.bFireAndForget = true;
	Info.bUseLargeFont = false;
	Info.bUseThrobber = false;
	Info.ExpireDuration = 5.0f;
	Info.bUseSuccessFailIcons = true;

	const auto NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
	if (!NotificationItem.IsValid()) { return; }

	NotificationItem->SetCompletionState(SNotificationItem::CS_Fail);
}

void FGameDataEditorToolkit::OnBrowserConsoleMessage(const FString& Message, const FString& Source, int32 Line,
	EWebBrowserConsoleLogSeverity Severity)
{
	switch (Severity)
	{
		case EWebBrowserConsoleLogSeverity::Debug: UE_LOG(LogFGameDataEditorToolkit, Verbose, TEXT("[%s:%d] %s."), *Source, Line, *Message); break;
		case EWebBrowserConsoleLogSeverity::Warning: UE_LOG(LogFGameDataEditorToolkit, Warning, TEXT("[%s:%d] %s."), *Source, Line, *Message); break;
		case EWebBrowserConsoleLogSeverity::Fatal:
		case EWebBrowserConsoleLogSeverity::Error: UE_LOG(LogFGameDataEditorToolkit, Error, TEXT("[%s:%d] %s."), *Source, Line, *Message); break;
		default:  UE_LOG(LogFGameDataEditorToolkit, Log, TEXT("[%s:%d] %s."), *Source, Line, *Message); break;

	}
	
}