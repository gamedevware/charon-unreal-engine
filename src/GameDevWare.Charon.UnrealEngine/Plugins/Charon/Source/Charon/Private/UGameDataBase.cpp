// Copyright GameDevWare, Denis Zykov 2025

#include "GameData/UGameDataBase.h"

#include "EditorFramework/AssetImportData.h"
#include "GameData/UGameDataImportData.h"

void UGameDataBase::PostInitProperties()
{
#if WITH_EDITORONLY_DATA
	if (!HasAnyFlags(RF_ClassDefaultObject | RF_NeedLoad))
	{
		AssetImportData = NewObject<UGameDataImportData>(this, TEXT("GameDataImportData"));
		AssetImportData->AddFileName(FString(), 0); // add empty filename for now
	}
#endif
	Super::PostInitProperties();
}

void UGameDataBase::PostLoad()
{
	Super::PostLoad();

#if WITH_EDITORONLY_DATA
	if (Cast<UGameDataImportData>(AssetImportData) == nullptr)
	{
		AssetImportData = NewObject<UGameDataImportData>(this, TEXT("GameDataImportData"));
	}
#endif
}

#if UE_VERSION_NEWER_THAN(5, 4, -1)
void UGameDataBase::GetAssetRegistryTags(FAssetRegistryTagsContext Context) const
{
	Super::GetAssetRegistryTags(Context);

#if WITH_EDITORONLY_DATA
	if (AssetImportData)
	{
		Context.AddTag(FAssetRegistryTag(SourceFileTagName(), AssetImportData->GetSourceData().ToJson(), FAssetRegistryTag::TT_Hidden));
	}
#endif
}
#endif
