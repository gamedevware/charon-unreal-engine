// Copyright GameDevWare, Denis Zykov 2024

#pragma once

#include "Framework/Commands/Commands.h"
#include "Templates/SharedPointer.h"

class CHARONEDITOR_API FGameDataEditorCommands: public TCommands< FGameDataEditorCommands >
{
public:
	FGameDataEditorCommands()
	: TCommands(
		TEXT("GameDataEditor"),
		NSLOCTEXT("Contexts", "GameDataEditor", "Game Data Editor"),
		NAME_None, // Parent
		FAppStyle::GetAppStyleSetName())
	{
	}
	
	virtual void RegisterCommands() override;

	TSharedPtr<FUICommandInfo> BrowserBack;
	TSharedPtr<FUICommandInfo> BrowserForward;
	TSharedPtr<FUICommandInfo> BrowserReload;
	
	TSharedPtr<FUICommandInfo> GenerateSourceCode;
	TSharedPtr<FUICommandInfo> Reimport;
	

	TSharedPtr<FUICommandInfo> Synchronize;
	TSharedPtr<FUICommandInfo> Connect;
	TSharedPtr<FUICommandInfo> Disconnect;
	TSharedPtr<FUICommandInfo> SetApiKey;
};
