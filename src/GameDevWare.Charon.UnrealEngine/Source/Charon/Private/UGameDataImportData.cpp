#include "GameData/UGameDataImportData.h"
#include "Misc/Paths.h"

bool UGameDataImportData::IsConnected() const
{
#if WITH_EDITORONLY_DATA
	return !ServerAddress.IsEmpty() && !ProjectId.IsEmpty() && !BranchId.IsEmpty();
#else
	return false;
#endif
}

void UGameDataImportData::Disconnect()
{
#if WITH_EDITORONLY_DATA
	ServerAddress.Empty();
	ProjectId.Empty();
	ProjectName.Empty();
	BranchId.Empty();
	BranchName.Empty();
#endif
}

FString UGameDataImportData::GetNormalizedGameDataPath() const
{
#if WITH_EDITOR
	FString GameDataFilePath = this->GetFirstFilename();
	if (GameDataFilePath.IsEmpty())
	{
		return GameDataFilePath;
	}
	GameDataFilePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir(), GameDataFilePath);
	FPaths::NormalizeFilename(GameDataFilePath);
	FPaths::CollapseRelativeDirectories(GameDataFilePath);
	return GameDataFilePath;
#endif
	return TEXT("");
}


