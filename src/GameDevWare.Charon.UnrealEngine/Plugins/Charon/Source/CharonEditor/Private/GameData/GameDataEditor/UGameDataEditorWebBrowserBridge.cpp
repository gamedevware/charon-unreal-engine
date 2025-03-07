// Copyright GameDevWare, Denis Zykov 2025

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

FString UGameDataEditorWebBrowserBridge::Upload(FString FileFilter)
{
	if (FileFilter.IsEmpty())
	{
		FileFilter = TEXT("All Files (*.*)|*.*");
	}

	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform)
	{
		return FString();
	}
	
	TArray<FString> OpenFileNames;
	auto bDialogResult = DesktopPlatform->OpenFileDialog(
		FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
		"Open",
		FEditorDirectories::Get().GetLastDirectory(ELastDirectory::GENERIC_OPEN),
		FString(),
		FileFilter,
		EFileDialogFlags::None,
		OpenFileNames
	);

	if (bDialogResult && OpenFileNames.Num() <= 0)
	{
		return FString();
	}

	TArray<uint8> ContentBytes;
	FString FileName = OpenFileNames[0];
	if (!FFileHelper::LoadFileToArray(ContentBytes, *FileName))
	{
		return FString();
	}

	FString MediaType = TEXT("application/octet-stream");
	const FString Extension = FPaths::GetExtension(FileName);
	if (FCString::Stricmp(*Extension, TEXT("GDJS")) == 0 || FCString::Stricmp(*Extension, TEXT("JSON")) == 0)
	{
		MediaType = TEXT("application/json");
	}
	else if (FCString::Stricmp(*Extension, TEXT("GDMP")) == 0 || FCString::Stricmp(*Extension, TEXT("MSGPACK")) == 0 ||
		FCString::Stricmp(*Extension, TEXT("MSGPCK")) == 0)
	{
		MediaType = TEXT("application/x-msgpack");
	}
	else if (FCString::Stricmp(*Extension, TEXT("GDBS")) == 0 || FCString::Stricmp(*Extension, TEXT("BSON")) == 0)
	{
		MediaType = TEXT("application/bson");
	}
	else if (FCString::Stricmp(*Extension, TEXT("xliff1")) == 0)
	{
		MediaType = TEXT("application/xliff+xml;version=1");
	}
	else if (FCString::Stricmp(*Extension, TEXT("xliff2")) == 0 || FCString::Stricmp(*Extension, TEXT("xlf")) == 0)
	{
		MediaType = TEXT("application/xliff+xml;version=2");
	}
	else if (FCString::Stricmp(*Extension, TEXT("xlsx")) == 0)
	{
		MediaType = TEXT("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");
	}
	
	FString ContentBase64 = FBase64::Encode(ContentBytes);
	ContentBase64.InsertAt(0, FString::Format(TEXT("data:{0};base64,"), { MediaType }));
	
	return ContentBase64;
}
