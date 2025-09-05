// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "CoreMinimal.h"
#include "EGameDataFormat.h"
#include "UGameDataDocument.h"
#include "UObject/Class.h"
#include "Engine/DataAsset.h"
#include "GameData/UGameDataImportData.h"

#include "UGameDataBase.generated.h"

/*
 * Base class for specific game data classes. Provides methods and data for UE editor extensions and stores import-related information.
 */
UCLASS(BlueprintType, Abstract, NotBlueprintable)
class CHARON_API UGameDataBase : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/*
	 * Import-related information including original path to game data file.
	 */
	UPROPERTY(VisibleAnywhere, Instanced, Category = ImportSettings)
	TObjectPtr<UGameDataImportData> AssetImportData;

	/*
	 * Try to load game data from specified game data file/stream using specified file format.
	 * Used for import. Patch-enabled load method is defined on derived class. 
	 */
	virtual bool TryLoad(FArchive* const GameDataStream, EGameDataFormat Format) { return false; }

	/*
	 * Find UGameDataDocument of specified SchemaNameOrId by DocumentId. Returns nullptr if document or schema is not found. Used by FGameDataDocumentReference.
	 */
	virtual UGameDataDocument* FindGameDataDocumentById(const FString& SchemaNameOrId, const FString& DocumentId) { return  nullptr; }
	/*
	 * Find document UClass by specified SchemaNameOrId. Used by FGameDataDocumentReferenceCustomization.
	 */
	virtual UClass* FindDocumentSchemaClass(const FString& SchemaNameOrId) { return  nullptr; }
	/*
	 * Get all document ids by specified SchemaNameOrId. Used by FGameDataDocumentReferenceCustomization.
	 */
	virtual void GetDocumentIds(const FString& SchemaNameOrId, TArray<FString>& OutAllIds) { }
	/*
	 * Get all schema names defined in derived game data class. Used by FGameDataDocumentReferenceCustomization.
	 */
	virtual void GetDocumentSchemaNames(TArray<FString>& OutAllSchemaNames) { }
	/*
	 * Make sure only specified languages are left in this game data.
	 */
	virtual void SetSupportedLanguages(const TArray<FString>& LanguageIds) { }
	/*
	 * Get revision hash of loaded game
	 */
	virtual FStringView GetRevisionHash()
	{
		// Use reflection to get the RevisionHash property
		UClass* Class = GetClass();
		if (FStrProperty* RevisionHashProperty = CastField<FStrProperty>(Class->FindPropertyByName(TEXT("RevisionHash"))))
		{
			// Get the value of the RevisionHash property from this object
			if (FString* RevisionHashValue = RevisionHashProperty->ContainerPtrToValuePtr<FString>(this))
			{
				return FStringView(*RevisionHashValue);
			}
		}
    
		// fallback to empty value 
		return FStringView();
	}
	
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;
};