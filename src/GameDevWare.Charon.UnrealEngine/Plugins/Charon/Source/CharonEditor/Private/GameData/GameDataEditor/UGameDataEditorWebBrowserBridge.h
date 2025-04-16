// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "FGameDataEditorToolkit.h"
#include "FListAssetsResult.h"

#include "UGameDataEditorWebBrowserBridge.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUGameDataEditorWebBrowserBridge, Log, All);

UCLASS()
class CHARONEDITOR_API UGameDataEditorWebBrowserBridge : public UObject
{
	GENERATED_BODY()
	
public:
	TWeakPtr<FGameDataEditorToolkit> EditorToolkit;
	
	UFUNCTION()
	bool Publish(FString Format, TArray<FString> Languages);
	
	UFUNCTION()
	bool GenerateSourceCode();

	UFUNCTION()
	bool Download(FString FileName, FString ContentBase64);

	UFUNCTION()
	FString Upload(FString FileFilter);

	UFUNCTION()
	FString ListAssets(int32 Skip, int32 Take, FString Query, TArray<FString> Types);

	UFUNCTION()
	FString GetAssetThumbnail(FString Path, int32 Size);
};

