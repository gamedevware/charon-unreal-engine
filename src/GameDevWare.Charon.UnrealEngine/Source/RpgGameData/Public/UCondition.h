
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

#include "UConditionsCheckFormula.h"
#include "UCondition.generated.h"


/**
  * Conditions with Text id.
  */
UCLASS(BlueprintType)
class RPGGAMEDATA_API UCondition : public UGameDataDocument
{
	GENERATED_BODY()

public:
    static const FString SchemaId;
    static const FString SchemaName;

public:
	/**
	  * Id property of Text type. Not Empty, Unique.
	  */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FString Id;
	/**
	  * Description property of Localized Text type. Not Null.
	  */
	UFUNCTION(BlueprintCallable)
	FText GetDescription();
	/**
	  * Raw value of Description.
	  */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FLocalizedText DescriptionRaw;
	/**
	  * Check property of Formula type. Not Null.
	  */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	UConditionsCheckFormula* Check;
	/**
	  * Unbound Check property of Formula type. Can Be Null.
	  */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FJsonObjectWrapper UnboundCheck;
};
