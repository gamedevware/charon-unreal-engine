// Copyright GameDevWare, Denis Zykov 2024

#pragma once
#include "FGameDataEditorToolkit.h"

#include "UGameDataEditorWebBrowserBridge.generated.h"

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
};

