// Copyright GameDevWare, Denis Zykov 2024

#pragma once
#include "FAuthenticationFlowStageResponse.h"
#include "FProjectsResponse.h"
#include "Interfaces/IHttpRequest.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFServerApiClient, Log, All);

DECLARE_DELEGATE_OneParam(OnGetMyProjectsResponse, FProjectsResponse)
DECLARE_DELEGATE_OneParam(OnGetLoginCodeResponse, FAuthenticationFlowStageResponse)

class FServerApiClient
{
	FString BaseAddress;
	TMap<FString, FString> RequestHeaders;
	TArray<TSharedRef<IHttpRequest>> PendingRequests;
	
public:
	
	// inline static FString DEFAULT_SERVER_ADDRESS = "http://localhost:3000";
	inline static FString DEFAULT_SERVER_ADDRESS = "https://charon.live";
	
	FServerApiClient(const FString& BaseAddress);

	FString GetApiKeysUrl() const;
	FString GetGameDataUrl(FString ProjectId, FString BranchId) const;
	
	void UseApiKey(const FString& ApiKey);

	bool GetMyProjects(OnGetMyProjectsResponse OnResponse);
	bool GetLoginCode(const FString& ApiKey, OnGetLoginCodeResponse OnResponse);
private:
	template<typename ObjectType>
	static bool TryReadJson(const FString& JsonText, ObjectType& OutObject);
	template<typename ObjectType>
	static bool TryWriteJson(ObjectType& InObject, TArray<uint8>& Output);
};
