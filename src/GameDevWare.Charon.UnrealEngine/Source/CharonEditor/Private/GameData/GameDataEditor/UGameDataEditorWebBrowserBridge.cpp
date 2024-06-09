// Copyright GameDevWare, Denis Zykov 2024

#pragma once

#include "UGameDataEditorWebBrowserBridge.h"

#include "DesktopPlatformModule.h"
#include "EditorDirectories.h"

// ReSharper disable CppMemberFunctionMayBeConst, CppParameterNeverUsed

bool UGameDataEditorWebBrowserBridge::Publish(FString Format, TArray<FString> Languages)
{
	const auto EditorToolkitPtr = EditorToolkit.Pin();
	if (!EditorToolkitPtr.IsValid()) { return false; }
	EditorToolkitPtr->Sync_Execute();
	return true;
}

bool UGameDataEditorWebBrowserBridge::GenerateSourceCode()
{
	const auto EditorToolkitPtr = EditorToolkit.Pin();
	if (!EditorToolkitPtr.IsValid()) { return false; }
	EditorToolkitPtr->GenerateSourceCode_Execute();
	return true;
}

bool UGameDataEditorWebBrowserBridge::Download(FString FileName, FString ContentBase64)
{
	int Base64Start = ContentBase64.Find("base64,");
	if (Base64Start < 0)
	{
		return false;
	}
	ContentBase64 = ContentBase64.RightChop(Base64Start + 7 /* "base64," */);

	TArray<uint8> ContentBytes;
	if (!FBase64::Decode(ContentBase64, ContentBytes))
	{
		return false;
	}

	if (IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get())
	{
		TArray<FString> SaveFileNames;
		auto bDialogResult = DesktopPlatform->SaveFileDialog(
			FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
			"Save As",
			FEditorDirectories::Get().GetLastDirectory(ELastDirectory::GENERIC_SAVE),
			FileName,
			TEXT("All Files (*.*)|*.*"),
			EFileDialogFlags::None,
			SaveFileNames
		);

		if (bDialogResult && SaveFileNames.Num() > 0)
		{
			return FFileHelper::SaveArrayToFile(ContentBytes, *SaveFileNames[0]);
		}
	}

	return false;
}
