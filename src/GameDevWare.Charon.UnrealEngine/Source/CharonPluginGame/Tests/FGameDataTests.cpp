// Copyright GameDevWare, Denis Zykov 2025

#if WITH_TESTS

#include "MathUtil.h"
#include "UTestData.h"
#include "Tests/TestHarnessAdapter.h"

TEST_CASE_NAMED(FGameDataTests, "Charon::TestGameData", "[Core]")
{
	SECTION("Loading JSON")
	{
		auto GameData = NewObject<UTestData>();

		auto GameDataFilePath = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("TestData.gdjs"));
		const TUniquePtr<FArchive> GameDataStream(IFileManager::Get().CreateFileReader(*GameDataFilePath, 0));
		
		REQUIRE(GameData->TryLoad(GameDataStream.Get(), EGameDataFormat::Json));

		CHECK(GameData->SupportedLanguages.Num() != 0);
		CHECK(GameData->RevisionHash.Len() != 0);
		CHECK(GameData->ChangeNumber != 0);
		CHECK(GameData->AllTestEntities.Num() == 3);
		CHECK(GameData->AllRecursiveEntities.Num() == 6);
		CHECK(GameData->AllNumberTestEntities.Num() == 4);
		CHECK(GameData->AllUniqueAttributeEntities.Num() == 1);

		GameData->SetLanguage(TEXT("fr-FR"), TEXT("fr-FR"), ELocalizedTextFallbackOption::OnNotDefined);

		CHECK(GameData->GetAllDocuments().Num() != 0);
		CHECK(GameData->GetRootDocuments().Num() != 0);

		const auto TestEntity = GameData->TestEntities.FindRef(ETestEntityId::TestEntity1);
		REQUIRE(TestEntity != nullptr);

		CHECK(TestEntity->NumberField == 1);
		CHECK(TestEntity->NumberFieldX64 == 9007199254740992);
		CHECK(TestEntity->IntegerField == 2147483647);
		CHECK(TestEntity->IntegerFieldX64 == 9223372036854775807);
		FDateTime ExpectedDate;
		FDateTime::ParseIso8601(TEXT("2017-05-31"), ExpectedDate);
		CHECK(TestEntity->DateField == ExpectedDate);
		FTimespan ExpectedTime;
		FTimespan::Parse(TEXT("00:00:01"), ExpectedTime);
		CHECK(TestEntity->TimeField == ExpectedTime);
		REQUIRE(TestEntity->DocumentField != nullptr);
		CHECK(TestEntity->DocumentField->NumberField + 0.1 < FMathf::Epsilon);
		CHECK(TestEntity->DocumentField->NumberFieldX64 == -9007199254740992);
		CHECK(TestEntity->DocumentField->IntegerField == static_cast<int32_t>(-2147483648LL));
		CHECK(TestEntity->DocumentField->IntegerFieldX64 == -9223372036854775807 - 1);
	}

	SECTION("Loading MessagePack")
	{
		auto GameData = NewObject<UTestData>();

		auto GameDataFilePath = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("TestData.gdmp"));
		const TUniquePtr<FArchive> GameDataStream(IFileManager::Get().CreateFileReader(*GameDataFilePath, 0));
		
		REQUIRE(GameData->TryLoad(GameDataStream.Get(), EGameDataFormat::MessagePack));

		CHECK(GameData->SupportedLanguages.Num() != 0);
		CHECK(GameData->RevisionHash.Len() != 0);
		CHECK(GameData->ChangeNumber != 0);
		CHECK(GameData->AllTestEntities.Num() == 3);
		CHECK(GameData->AllRecursiveEntities.Num() == 6);
		CHECK(GameData->AllNumberTestEntities.Num() == 4);
		CHECK(GameData->AllUniqueAttributeEntities.Num() == 1);

		GameData->SetLanguage(TEXT("fr-FR"), TEXT("fr-FR"), ELocalizedTextFallbackOption::OnNotDefined);

		CHECK(GameData->GetAllDocuments().Num() != 0);
		CHECK(GameData->GetRootDocuments().Num() != 0);
		
		const auto TestEntity = GameData->TestEntities.FindRef(ETestEntityId::TestEntity1);
        REQUIRE(TestEntity != nullptr);

		CHECK(TestEntity->NumberField == 1);
		CHECK(TestEntity->NumberFieldX64 == 9007199254740992);
		CHECK(TestEntity->IntegerField == 2147483647);
		CHECK(TestEntity->IntegerFieldX64 == 9223372036854775807);
		FDateTime ExpectedDate;
		FDateTime::ParseIso8601(TEXT("2017-05-31"), ExpectedDate);
		CHECK(TestEntity->DateField == ExpectedDate);
		FTimespan ExpectedTime;
		FTimespan::Parse(TEXT("00:00:01"), ExpectedTime);
		CHECK(TestEntity->TimeField == ExpectedTime);
		REQUIRE(TestEntity->DocumentField != nullptr);
		CHECK(TestEntity->DocumentField->NumberField + 0.1 < FMathf::Epsilon);
		CHECK(TestEntity->DocumentField->NumberFieldX64 == -9007199254740992);
		CHECK(TestEntity->DocumentField->IntegerField == static_cast<int32_t>(-2147483648LL));
		CHECK(TestEntity->DocumentField->IntegerFieldX64 == -9223372036854775807 - 1);
	}

	SECTION("Patching JSON")
	{
		auto GameData = NewObject<UTestData>();

		auto GameDataFilePath = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("TestData.gdjs"));
		auto GameDataPatch1Path = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("TestData.patch"));
		auto GameDataPatch2Path = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("TestData.patch2"));
		const TUniquePtr<FArchive> GameDataStream(IFileManager::Get().CreateFileReader(*GameDataFilePath, 0));
		const TUniquePtr<FArchive> GameDataPatch1Stream(IFileManager::Get().CreateFileReader(*GameDataPatch1Path, 0));
		const TUniquePtr<FArchive> GameDataPatch2Stream(IFileManager::Get().CreateFileReader(*GameDataPatch2Path, 0));

		auto Options = FGameDataLoadOptions();
		Options.Format = EGameDataFormat::Json;
		Options.Patches.Push(GameDataPatch1Stream.Get());
		Options.Patches.Push(GameDataPatch2Stream.Get());
		
		REQUIRE(GameData->TryLoad(GameDataStream.Get(), Options));

		const auto ProjectSettings = GameData->ProjectSettings;

		REQUIRE(ProjectSettings != nullptr);
		CHECK(ProjectSettings->Version == TEXT("0.0.0.1"));

		CHECK(GameData->SupportedLanguages.Contains(TEXT("en-US")));
		CHECK(GameData->SupportedLanguages.Contains(TEXT("ru-RU")));
		CHECK(GameData->SupportedLanguages.Contains(TEXT("en-GB")));
		CHECK(GameData->SupportedLanguages.Contains(TEXT("fr-FR")));

		const auto TestEntity = GameData->TestEntities.FindRef(ETestEntityId::TestEntity1);
		REQUIRE(TestEntity != nullptr);

		CHECK(TestEntity->TextField == TEXT("Patched2"));

		CHECK(TestEntity->LocalizedTextFieldRaw.TextByLanguageId.FindRef(TEXT("fr-FR")).EqualTo(INVTEXT("Patched2")));
		CHECK(TestEntity->LocalizedTextFieldRaw.TextByLanguageId.FindRef(TEXT("en-GB")).EqualTo(INVTEXT("Patched1")));
		CHECK(TestEntity->LocalizedTextFieldRaw.TextByLanguageId.FindRef(TEXT("ru-RU")).EqualTo(INVTEXT("Patched1")));

		CHECK(TestEntity->DocumentField == nullptr);
		CHECK(TestEntity->ListOfDocumentsField.Num() == 0);

		CHECK(TestEntity->NumberField == 1);
		CHECK(TestEntity->NumberFieldX64 == 9007199254740992);
		CHECK(TestEntity->IntegerField == 2147483647);
		CHECK(TestEntity->IntegerFieldX64 == 9223372036854775807);
		FDateTime ExpectedDate;
		FDateTime::ParseIso8601(TEXT("2017-05-31"), ExpectedDate);
		CHECK(TestEntity->DateField == ExpectedDate);
		FTimespan ExpectedTime;
		FTimespan::Parse(TEXT("00:00:01"), ExpectedTime);
		CHECK(TestEntity->TimeField == ExpectedTime);
		CHECK(TestEntity->DocumentField == nullptr);
	}
}

#endif //WITH_TESTS