
#pragma once
//
// The source code was generated by the Charon, GameDevWare, 2025
// License: MIT
//
//------------------------------------------------------------------------------
// <auto-generated>
//	 This code was generated by a tool.
//	 Changes to this file may cause incorrect behavior and will be lost if
//	 the code is regenerated.
// </auto-generated>
//------------------------------------------------------------------------------
// ReSharper disable All

#include "GameData/UGameDataDocument.h"
#include "GameData/FGameDataDocumentReference.h"
#include "GameData/FLocalizedText.h"

#include "EMonsterType.h"
#include "UMonster.generated.h"

class UParameterValue; // forward declaration
class ULocation; // forward declaration

/**
  * Monsters with Text id.
  */
UCLASS(BlueprintType)
class RPGGAMEDATA_API UMonster : public UGameDataDocument
{
	GENERATED_BODY()

public:
    static const FString SchemaId;
    static const FString SchemaName;

	/**
	  * De-referenced collection of documents for Locations. Should not be directly used.
	  */
	TMap<FString,ULocation*> _locationsDocuments;
public:
	/**
	  * Id property of Text type. Not Empty, Unique.
	  */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FString Id;
	/**
	  * Name property of Localized Text type. Not Null.
	  */
	UFUNCTION(BlueprintCallable)
	FText GetName();
	/**
	  * Raw value of Name.
	  */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FLocalizedText NameRaw;
	/**
	  * Type property of Pick List type. Not Null.
	  */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	EMonsterType Type;
	/**
	  * Second Type property of Pick List type. Can Be Null.
	  */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	EMonsterType SecondType;
	/**
	  * Parameters property of Document Collection type. Not Null.
	  */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TMap<int32,UParameterValue*> Parameters;
	/**
	  * Locations property of Reference Collection type. Not Null.
	  */
	UFUNCTION(BlueprintCallable)
	TMap<FString,ULocation*> GetLocations();
	/**
	  * Raw value of Locations.
	  */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FGameDataDocumentReference> LocationsRaw;
};
