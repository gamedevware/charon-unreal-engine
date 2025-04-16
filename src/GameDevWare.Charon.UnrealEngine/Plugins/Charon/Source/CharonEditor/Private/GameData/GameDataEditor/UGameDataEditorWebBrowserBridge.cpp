﻿// Copyright GameDevWare, Denis Zykov 2025

#include "UGameDataEditorWebBrowserBridge.h"

#include "DesktopPlatformModule.h"
#include "EditorDirectories.h"
#include "IImageWrapper.h"
#include "JsonObjectConverter.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ThumbnailRendering/ThumbnailManager.h"

// ReSharper disable CppMemberFunctionMayBeConst, CppParameterNeverUsed
DEFINE_LOG_CATEGORY(LogUGameDataEditorWebBrowserBridge);

bool UGameDataEditorWebBrowserBridge::Publish(FString Format, TArray<FString> Languages)
{
	const auto EditorToolkitPtr = EditorToolkit.Pin();
	if (!EditorToolkitPtr.IsValid()) { return false; }

	if (Languages.Num() == 1 && Languages[0] == TEXT("*"))
	{
		Languages.Empty(); // empty languages = all languages
	} 

	for (FString& LanguageId : Languages)
	{
		if (LanguageId.Len() >= 2 && LanguageId.StartsWith("{") && LanguageId.EndsWith("}"))
		{
			LanguageId.MidInline(1, LanguageId.Len() - 2);
		}
	}
	
	for (auto EditingObjectPtr : EditorToolkitPtr->GetEditingObjectPtrs())
	{
		const UGameDataBase* GameDataPtr = Cast<UGameDataBase>(EditingObjectPtr);
		if (!GameDataPtr) continue;
		GameDataPtr->AssetImportData->PublishLanguages = Languages;
	}
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

	if (!bDialogResult || OpenFileNames.Num() <= 0)
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

FString UGameDataEditorWebBrowserBridge::ListAssets(int32 Skip, int32 Take, FString Query,
	TArray<FString> Types)
{
	FListAssetsResult Result;
	Result.Total = 0;
	Result.Assets.Empty();

	// Get the Asset Registry module
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Convert string class names to UClass* (e.g., "Texture2D" -> UTexture2D::StaticClass())
	TArray<UClass*> FilterClasses;
	for (const FString& TypeName : Types)
	{
		if (UClass* Class = FindFirstObject<UClass>(*TypeName, EFindFirstObjectOptions::ExactClass))
		{
			FilterClasses.Add(Class);
		}
	}

	if (FilterClasses.Num() == 0)
	{
		FilterClasses.Add(UObject::StaticClass());
	}
	
	// Build the filter
	FARFilter Filter;
	if (FilterClasses.Num() > 0)
	{
		Filter.ClassPaths.Append(FilterClasses);
	}
	Filter.bRecursiveClasses = true; // Include subclasses
	Filter.bIncludeOnlyOnDiskAssets = true; // Include persisted game assets
	Filter.RecursiveClassPathsExclusionSet.Emplace(FTopLevelAssetPath(FString(TEXT("/Engine"))));
	Filter.RecursiveClassPathsExclusionSet.Emplace(FTopLevelAssetPath(FString(TEXT("/Script"))));
	
	// Get all assets matching the filter
	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssets(Filter, AssetDataList);
	AssetDataList.RemoveAll([Query](const FAssetData& InValue)
	{
		const FString AssetName = InValue.GetObjectPathString();
		const bool bIsMatching = Query.IsEmpty() || AssetName.Contains(Query, ESearchCase::IgnoreCase);
		const bool bIsBuildIn =  AssetName.StartsWith(TEXT("/Engine")) || AssetName.StartsWith(TEXT("/Script"));
		return !bIsMatching || bIsBuildIn;
	});

	// Apply pagination (Skip/Take)
	Result.Total = AssetDataList.Num();
	const int32 EndIndex = FMath::Min(Skip + Take, AssetDataList.Num());

	for (int32 i = Skip; i < EndIndex; ++i)
	{
		const FAssetData& AssetData = AssetDataList[i];
		FGameAsset GameAsset;

		// Fill asset info
		GameAsset.Name = AssetData.AssetName.ToString();
		GameAsset.Path = AssetData.GetObjectPathString();
		GameAsset.Type = FPaths::GetCleanFilename(AssetData.AssetClassPath.ToString());

		// Check if the asset has a thumbnail (optional)
		GameAsset.HasThumbnail = AssetData.IsUAsset(); // Or use a more advanced check

		Result.Assets.Add(GameAsset);
	}

	FString ResultAsJsonString;
	FJsonObjectConverter::UStructToJsonObjectString(Result, ResultAsJsonString);

	return ResultAsJsonString;
}

TMap<FString, FString> ThumbnailCache; 
FString UGameDataEditorWebBrowserBridge::GetAssetThumbnail(FString Path, int32 Size)
{
	// Check cache first
	FString CacheKey = FString::Printf(TEXT("%s_%d"), *Path, Size);
	if (FString* CachedThumbnail = ThumbnailCache.Find(CacheKey))
	{
		return *CachedThumbnail;
	}

    // 1. Load the asset from the path
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(Path));

    if (!AssetData.IsValid())
    {
        return TEXT(""); // Invalid path
    }

    // 2. Get the thumbnail image
    UObject* Asset = AssetData.GetAsset();
    if (!Asset)
    {
        return TEXT(""); // Failed to load
    }

    FThumbnailRenderingInfo* ThumbnailInfo = UThumbnailManager::Get().GetRenderingInfo(Asset);
    if (!ThumbnailInfo)
    {
        return TEXT(""); // No thumbnail available
    }

	
    // 3. Render the thumbnail
	FTextureRenderTargetResource* RenderTargetResource = nullptr;
	UTextureRenderTarget2D* RenderTargetTexture = GEditor->GetScratchRenderTarget( Size );
    if (!RenderTargetTexture)
    {
        return TEXT("");
    }
	RenderTargetResource = RenderTargetTexture->GameThread_GetRenderTargetResource();
	
	FObjectThumbnail NewThumbnail;
	ThumbnailTools::RenderThumbnail(Asset, Size, Size, ThumbnailTools::EThumbnailTextureFlushMode::AlwaysFlush, RenderTargetResource, &NewThumbnail);


	NewThumbnail.CompressImageData();
	
    TArray<uint8>& CompressedData = NewThumbnail.AccessCompressedImageData();
	
    // 6. Convert to Data URL
	FString MediaType = NewThumbnail.GetCompressor()->IsLosslessCompression() ? TEXT("image/png") : TEXT("image/jpeg");  
    FString ThumbnailDataUrl = FBase64::Encode(CompressedData);
	ThumbnailDataUrl.InsertAt(0, FString::Format(TEXT("data:{0};base64,"), { MediaType }));

	// Cache for future calls
	if (!ThumbnailDataUrl.IsEmpty())
	{
		ThumbnailCache.Add(CacheKey, ThumbnailDataUrl);
	}

	return ThumbnailDataUrl;
}
