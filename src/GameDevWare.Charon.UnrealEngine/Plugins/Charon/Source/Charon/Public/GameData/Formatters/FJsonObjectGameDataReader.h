// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "IGameDataReader.h"

#include "Containers/Array.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Containers/UnrealString.h"
#include "Templates/SharedPointer.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonTypes.h"
#include "Misc/EngineVersionComparison.h"

/*
 * Token based reader for dynamic FJsonObject object as source of token stream.
 * Provides generalized IGameDataReader behaviour for already de-serialized objects.
 * Used for patching FJsonObject and TryLoad from resulting merged FJsonObject.
 */
class CHARON_API FJsonObjectGameDataReader final : public IGameDataReader
{
private:
	class FJsonObjectReaderFrame
	{
	private:
		bool MemberNameIsVisited;
		size_t CurrentIndex;
		TArray<TSharedPtr<FJsonValue>>* CurrentArray;
#if UE_VERSION_NEWER_THAN(5, 8, -1)
		TArray<UE::FSharedString> CurrentObjectKeys;
#else
		TArray<FString> CurrentObjectKeys;
#endif
		TSharedPtr<FJsonObject>* CurrentObject;

	public:
		explicit FJsonObjectReaderFrame(TSharedPtr<FJsonObject>& JsonObject);
		explicit FJsonObjectReaderFrame(TArray<TSharedPtr<FJsonValue>>& JsonObject);

#if UE_VERSION_NEWER_THAN(5, 8, -1)
		UE::FSharedString& GetCurrentMemberName();
#else
		FString& GetCurrentMemberName();
#endif
		
		TSharedPtr<FJsonValue> GetCurrentValue();
		EJson GetContainerToken() const;

		void NextMemberOrItem();
		EJsonToken GetNextJsonToken();
	};

	TSharedPtr<FJsonObject> JsonObject;
	TArray<FJsonObjectReaderFrame> Frames;
	EJsonNotation Notation;
	EJsonToken CurrentToken;
	FString Identifier;
	FString ErrorMessage;
	FString StringValue;
	double NumberValue;
	bool BoolValue;
	bool FinishedReadingRootObject;

public:
	explicit FJsonObjectGameDataReader(const TSharedPtr<FJsonObject>& JsonObject);

	virtual bool ReadNext() override;
	virtual const FString& GetIdentifier() const override;
	virtual const FString GetValueAsString() const override;
	virtual double GetValueAsNumber() const override;
	virtual bool GetValueAsBoolean() const override;
	virtual const FString& GetErrorMessage() const override;
	virtual EJsonNotation GetNotation() override;
	virtual void SetErrorState(const FString& Message) override;

	// TODO Should override SkipAny for efficient skipping big JsonObject trees
};
