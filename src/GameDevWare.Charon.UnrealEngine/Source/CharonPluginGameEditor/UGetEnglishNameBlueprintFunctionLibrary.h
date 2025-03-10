#pragma once
//------------------------------------------------------------------------------
// <auto-generated>
//	 This code was generated by a tool.
//	 Changes to this file may cause incorrect behavior and will be lost if
//	 the code is regenerated.
// </auto-generated>
//------------------------------------------------------------------------------
#include "UArmor.h"
#include "UCurio.h"
#include "UDisease.h"
#include "UHero.h"
#include "UItem.h"
#include "ULocation.h"
#include "UMonster.h"
#include "UQuirk.h"
#include "UWeapon.h"

#include "UGetEnglishNameBlueprintFunctionLibrary.generated.h"

UCLASS()
class UGetEnglishNameBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	UFUNCTION(BlueprintCallable)
	static FText GetArmorName(UArmor* Document)
	{
		if (Document == nullptr)
		{
			return FText::GetEmpty();
		}

		FText* Value = Document->NameRaw.TextByLanguageId.Find(TEXT("en-US"));
		if (!Value)
		{
			return FText::GetEmpty();
		}

		return *Value;
	}
	UFUNCTION(BlueprintCallable)
	static FText GetCurioName(UCurio* Document)
	{
		if (Document == nullptr)
		{
			return FText::GetEmpty();
		}

		FText* Value = Document->NameRaw.TextByLanguageId.Find(TEXT("en-US"));
		if (!Value)
		{
			return FText::GetEmpty();
		}

		return *Value;
	}
	UFUNCTION(BlueprintCallable)
	static FText GetDiseaseName(UDisease* Document)
	{
		if (Document == nullptr)
		{
			return FText::GetEmpty();
		}

		FText* Value = Document->NameRaw.TextByLanguageId.Find(TEXT("en-US"));
		if (!Value)
		{
			return FText::GetEmpty();
		}

		return *Value;
	}
	UFUNCTION(BlueprintCallable)
	static FText GetHeroName(UHero* Document)
	{
		if (Document == nullptr)
		{
			return FText::GetEmpty();
		}

		FText* Value = Document->NameRaw.TextByLanguageId.Find(TEXT("en-US"));
		if (!Value)
		{
			return FText::GetEmpty();
		}

		return *Value;
	}
	UFUNCTION(BlueprintCallable)
	static FText GetItemName(UItem* Document)
	{
		if (Document == nullptr)
		{
			return FText::GetEmpty();
		}

		FText* Value = Document->NameRaw.TextByLanguageId.Find(TEXT("en-US"));
		if (!Value)
		{
			return FText::GetEmpty();
		}

		return *Value;
	}
	UFUNCTION(BlueprintCallable)
	static FText GetLocationName(ULocation* Document)
	{
		if (Document == nullptr)
		{
			return FText::GetEmpty();
		}

		FText* Value = Document->NameRaw.TextByLanguageId.Find(TEXT("en-US"));
		if (!Value)
		{
			return FText::GetEmpty();
		}

		return *Value;
	}
	UFUNCTION(BlueprintCallable)
	static FText GetMonsterName(UMonster* Document)
	{
		if (Document == nullptr)
		{
			return FText::GetEmpty();
		}

		FText* Value = Document->NameRaw.TextByLanguageId.Find(TEXT("en-US"));
		if (!Value)
		{
			return FText::GetEmpty();
		}

		return *Value;
	}
	UFUNCTION(BlueprintCallable)
	static FText GetQuirkName(UQuirk* Document)
	{
		if (Document == nullptr)
		{
			return FText::GetEmpty();
		}

		FText* Value = Document->NameRaw.TextByLanguageId.Find(TEXT("en-US"));
		if (!Value)
		{
			return FText::GetEmpty();
		}

		return *Value;
	}
	UFUNCTION(BlueprintCallable)
	static FText GetWeaponName(UWeapon* Document)
	{
		if (Document == nullptr)
		{
			return FText::GetEmpty();
		}

		FText* Value = Document->NameRaw.TextByLanguageId.Find(TEXT("en-US"));
		if (!Value)
		{
			return FText::GetEmpty();
		}

		return *Value;
	}
};
