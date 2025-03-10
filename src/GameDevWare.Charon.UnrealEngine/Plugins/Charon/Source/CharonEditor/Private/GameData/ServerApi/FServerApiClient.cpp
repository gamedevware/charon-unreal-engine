﻿// Copyright GameDevWare, Denis Zykov 2025

// ReSharper disable CppPassValueParameterByConstReference

#include "FServerApiClient.h"

#include "FApiKeyAuthenticateRequest.h"
#include "FAuthenticationFlowStageResponse.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "JsonObjectConverter.h"
#include "Engine/EngineBaseTypes.h"

DEFINE_LOG_CATEGORY(LogFServerApiClient);

FServerApiClient::FServerApiClient(const FString& BaseAddress)
	: BaseAddress(BaseAddress)
{
	RequestHeaders.Add("User-Agent", "Charon, Unreal Editor, FServerApiClient");

	this->BaseAddress = this->BaseAddress.TrimChar('/').TrimChar('\\');
}

FString FServerApiClient::GetApiKeysUrl() const
{
	return BaseAddress + "/view/user/me/profile/api-keys";
}

FString FServerApiClient::GetGameDataUrl(FString ProjectId, FString BranchId) const
{
	return BaseAddress + FString::Format(TEXT("/view/data/{0}/{1}/"), { ProjectId, BranchId });
}

void FServerApiClient::UseApiKey(const FString& ApiKey)
{
	this->RequestHeaders.Remove("Authorization");
	this->RequestHeaders.Add("Authorization", "Bearer " + ApiKey);
}

bool FServerApiClient::GetMyProjects(OnGetMyProjectsResponse OnResponse)
{
	FHttpModule& HttpModule = FHttpModule::Get();
	const TSharedRef<IHttpRequest> Request = HttpModule.CreateRequest();
	Request->SetVerb(TEXT("GET"));
	for (auto RequestHeader : RequestHeaders)
	{
		Request->SetHeader(RequestHeader.Key, RequestHeader.Value);
	}
	Request->SetHeader("Accept", "application/json");
	Request->SetURL(BaseAddress + "/api/v1/project/my/");
	Request->OnProcessRequestComplete().BindLambda([OnResponse](
		FHttpRequestPtr RequestPtr,
		FHttpResponsePtr ResponsePtr,
		bool bConnectedSuccessfully)
		{
			auto ResponseObject = FProjectsResponse();
			if (bConnectedSuccessfully)
			{
				const auto ResponseJson = ResponsePtr->GetContentAsString();
				if (!TryReadJson(ResponseJson, ResponseObject))
				{
					ResponseObject.Errors.Add(FApiError{TEXT("Failed to deserialize JSON response"), TEXT("error")});
				}
			}
			else
			{
				switch (RequestPtr->GetStatus())
				{
				case EHttpRequestStatus::Failed:
					ResponseObject.Errors.Add(FApiError{TEXT("Connection failed."), TEXT("error")});
				default:
					ResponseObject.Errors.Add(FApiError{TEXT("Request failed."), TEXT("error")});
				}
			}
			auto _ = OnResponse.ExecuteIfBound(ResponseObject);
		});
	
	PendingRequests.Add(Request);
	return Request->ProcessRequest();
}

bool FServerApiClient::GetLoginCode(const FString& ApiKey, OnGetLoginCodeResponse OnResponse)
{
	FHttpModule& HttpModule = FHttpModule::Get();

	FApiKeyAuthenticateRequest RequestObject;
	RequestObject.ApiKey = ApiKey;
	TArray<uint8> RequestBody;
	if (!TryWriteJson(RequestObject, RequestBody))
	{
		return false;
	}
	
	const TSharedRef<IHttpRequest> Request = HttpModule.CreateRequest();
	Request->SetVerb(TEXT("POST"));
	for (auto RequestHeader : RequestHeaders)
	{
		Request->SetHeader(RequestHeader.Key, RequestHeader.Value);
	}
	Request->SetHeader("Accept", "application/json");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetURL(BaseAddress + "/api/v1/auth/flow/api-key/");
	Request->SetContent(RequestBody);
	Request->OnProcessRequestComplete().BindLambda([OnResponse](
		FHttpRequestPtr RequestPtr,
		FHttpResponsePtr ResponsePtr,
		bool bConnectedSuccessfully)
		{
			auto ResponseObject = FAuthenticationFlowStageResponse();
			if (bConnectedSuccessfully)
			{
				const auto ResponseJson = ResponsePtr->GetContentAsString();
				if (!TryReadJson(ResponseJson, ResponseObject))
				{
					ResponseObject.Errors.Add(FApiError{TEXT("Failed to deserialize JSON response"), TEXT("error")});
				}
			}
			else
			{
				switch (RequestPtr->GetStatus())
				{
				case EHttpRequestStatus::Failed:
					ResponseObject.Errors.Add(FApiError{TEXT("Connection failed."), TEXT("error")});
				default:
					ResponseObject.Errors.Add(FApiError{TEXT("Request failed."), TEXT("error")});
				}
			}
			auto _ = OnResponse.ExecuteIfBound(ResponseObject);
		});
	
	PendingRequests.Add(Request);
	return Request->ProcessRequest();
}

template <typename ObjectType>
bool FServerApiClient::TryReadJson(const FString& JsonText, ObjectType& OutObject)
{
	TSharedPtr<FJsonValue> OutValue;
	const auto JsonReader = TJsonReaderFactory<>::Create(JsonText);

	if (!FJsonSerializer::Deserialize(JsonReader, OutValue) || !OutValue.IsValid())
	{
		UE_LOG(LogFServerApiClient, Warning,
		       TEXT("Failed to read HTTP request result as JSON. Text is not a valid JSON. Text: "));
		UE_LOG(LogFServerApiClient, Warning, TEXT("%s"), *JsonText);
		return false;
	}

	TSharedPtr<FJsonObject>* OutJsonObject;
	if (!OutValue->TryGetObject(OutJsonObject) || !OutJsonObject->IsValid())
	{
		UE_LOG(LogFServerApiClient, Warning,
		       TEXT("Failed to read HTTP request result as JSON. Result value is not a JSON object."));
		return false;
	}

	FText FailReason;
	if (!FJsonObjectConverter::JsonObjectToUStruct<ObjectType>(OutJsonObject->ToSharedRef(), &OutObject, 0, 0, false,
	                                                           &FailReason))
	{
		UE_LOG(LogFServerApiClient, Warning,
		       TEXT("Failed to read HTTP request result as JSON. Failed to deserialize to UObject. Reason: %s."),
		       *FailReason.ToString());
		return false;
	}

	return true;
}

template <typename ObjectType>
bool FServerApiClient::TryWriteJson(ObjectType& InObject, TArray<uint8>& Output)
{
	FString OutputJson;
	if (!FJsonObjectConverter::UStructToJsonObjectString<ObjectType>(InObject, OutputJson, 0, 0, 0, nullptr, /* pretty */ true))
	{
		UE_LOG(LogFServerApiClient, Warning,
			   TEXT("Failed to write HTTP request body as JSON. Failed to serialize UObject."));
		return false;
	}

	const FTCHARToUTF8 Converter(*OutputJson);
	const int32 Length = Converter.Length();

	Output.SetNumUninitialized(Length);
	FMemory::Memcpy(Output.GetData(), Converter.Get(), Length);
	
	return true;
}
