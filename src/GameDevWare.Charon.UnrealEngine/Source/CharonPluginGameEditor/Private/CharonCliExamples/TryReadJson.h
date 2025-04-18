// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "GameDataExtensionCommandsLog.h"

static bool TryReadJson(const FString& JsonContent, TSharedPtr<FJsonObject>& OutResult)
{
	const auto JsonReader = TJsonReaderFactory<>::Create(JsonContent);
	TSharedPtr<FJsonValue> OutValue;

	if (!FJsonSerializer::Deserialize(JsonReader, OutValue) || !OutValue.IsValid())
	{
		UE_LOG(LogFGameDataExtensionCommands, Warning, TEXT("Failed to read content as JSON. Input is not valid JSON. Content: %s"), *JsonContent);
		return false;
	}

	TSharedPtr<FJsonObject>* OutObject;
	if (!OutValue->TryGetObject(OutObject) || !OutObject->IsValid())
	{
		UE_LOG(LogFGameDataExtensionCommands, Warning, TEXT("Failed to read content as JSON. Result value is not a JSON object."));
		return false;
	}

	check(OutObject);
	OutResult = OutObject->ToSharedRef();
	return true;
}

static bool TryReadJsonFile(const FString& FilePath, TSharedPtr<FJsonObject>& OutResult)
{
	FString JsonContent;
	if (!FFileHelper::LoadFileToString(JsonContent, *FilePath))
	{
		UE_LOG(LogFGameDataExtensionCommands, Warning, TEXT("Failed to read JSON file. File not found or could not be read: %s"), *FilePath);
		return false;
	}
	
	return TryReadJson(JsonContent, OutResult);
}