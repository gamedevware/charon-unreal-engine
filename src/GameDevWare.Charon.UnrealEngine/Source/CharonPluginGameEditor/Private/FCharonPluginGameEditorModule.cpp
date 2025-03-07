// Copyright GameDevWare, Denis Zykov 2025

#include "FCharonPluginGameEditorModule.h"
#include "CharonCliExamples/FGameDataExtensionCommands.h"
#include "GameData/FCharonEditorModule.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FCharonPluginGameEditorModule"

void FCharonPluginGameEditorModule::StartupModule()
{
	// retrieve instance of Charon Editor mode for extension
	// add "CharonEditor" to private dependencies in your .Build.cs file
	ICharonEditorModule& CharonEditorModule = FCharonEditorModule::Get();

	// don't forget register Commands before using them
	FGameDataExtensionCommands::Register();

	// Register menu extension for Game Data Editor window
	// Since we interested in ContextSensitiveObjects (UGameDataBase*) we use GetExtenderDelegates() way of registering extension
	TArray<FAssetEditorExtender>& ExtenderDelegateList = CharonEditorModule.GetGameDataEditorMenuExtensibilityManager()->GetExtenderDelegates();
	ExtenderDelegateList.Add(FAssetEditorExtender::CreateLambda([](const TSharedRef<FUICommandList>&, const TArray<UObject*>& ContextSensitiveObjects)
	{
		TSharedRef<FExtender> GameDataMenuExtender = MakeShareable(new FExtender);
		const auto GameDataExtensionCommands = FGameDataExtensionCommands::GetCommandList(ContextSensitiveObjects);
		
		GameDataMenuExtender->AddMenuExtension(
			"AssetEditorActions",
			EExtensionHook::After,
			GameDataExtensionCommands,
			FMenuExtensionDelegate::CreateStatic(&FGameDataExtensionCommands::ConstructGameDataExtensionMenu)
		);
		
		return GameDataMenuExtender;
	}));
}

void FCharonPluginGameEditorModule::ShutdownModule()
{
	FGameDataExtensionCommands::Unregister();
}


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCharonPluginGameEditorModule, CharonPluginGameEditor)
