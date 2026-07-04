// Copyright GameDevWare, Denis Zykov 2025

// ReSharper disable CppPassValueParameterByConstReference
#pragma once

#include "GameData/Formulas/EFormulaValueType.h"
#include "EStringSplitOptions.h"
#include "EStringComparison.h"
#include "Misc/CString.h"

#if __has_include("UObject/StrProperty.h")
#include "UObject/StrProperty.h"
#endif
#include "Misc/EngineVersionComparison.h"
#include "UDotNetString.generated.h"

/*
 * Internal struct used for reflection in formulas
 */
UCLASS(Hidden, NotPlaceable, Abstract)
class CHARON_API UDotNetString : public UObject
{
	GENERATED_BODY()

private:
	UPROPERTY()
	FString __Literal;
	UPROPERTY()
	TArray<FString> __ArrayLiteral;

#if UE_VERSION_NEWER_THAN(5, 6, -1)
	static ESearchCase::Type ToSearchCase(TOptional<EStringComparison> ComparisonType)
	{
		return ToSearchCase(ComparisonType.IsSet() ? ComparisonType.GetValue() : EStringComparison::Ordinal); 
	}
#endif
	static ESearchCase::Type ToSearchCase(EStringComparison ComparisonType)
	{
		return (ComparisonType == EStringComparison::OrdinalIgnoreCase
			 || ComparisonType == EStringComparison::CurrentCultureIgnoreCase
			 || ComparisonType == EStringComparison::InvariantCultureIgnoreCase)
			? ESearchCase::IgnoreCase : ESearchCase::CaseSensitive;
	}
	
public:
	inline static EFormulaValueType TypeCode = EFormulaValueType::String;

	static FStrProperty* GetLiteralProperty()
	{
		static FStrProperty* LiteralProperty = nullptr;

		if (LiteralProperty)
		{
			return LiteralProperty;
		}
		return LiteralProperty = CastFieldChecked<FStrProperty>(StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UDotNetString, __Literal)));
	}
	static FArrayProperty* GetArrayProperty()
	{
		static FArrayProperty* ArrayProperty = nullptr;

		if (ArrayProperty)
		{
			return ArrayProperty;
		}
		return ArrayProperty = CastFieldChecked<FArrayProperty>(StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UDotNetString, __ArrayLiteral)));
	}

	/* static properties */

	UPROPERTY()
	FString Empty = TEXT("");

	//* instance properties */

	UFUNCTION()
	static int GetLength(FString Self)
	{
		return Self.Len();
	}

	UFUNCTION()
	static int GetCount(FString Self)
	{
		return Self.Len();
	}
	
	//* instance methods */

	UFUNCTION()
	static int CompareTo(FString Self, FString StrB,
#if UE_VERSION_NEWER_THAN(5, 6, -1)
		TOptional<EStringComparison> ComparisonType
#else
		EStringComparison ComparisonType = EStringComparison::Ordinal
#endif
	)
	{
		if (ToSearchCase(ComparisonType) == ESearchCase::IgnoreCase)
			return FCString::Stricmp(*Self, *StrB);
		return FCString::Strcmp(*Self, *StrB);
	}

	UFUNCTION()
	static int Contains(FString Self, FString StrB, 
#if UE_VERSION_NEWER_THAN(5, 6, -1)
		TOptional<EStringComparison> ComparisonType
#else
		EStringComparison ComparisonType = EStringComparison::Ordinal
#endif
	)
	{
		return Self.Contains(*StrB, ToSearchCase(ComparisonType)) ? 1 : 0;
	}

	UFUNCTION()
	static bool EndsWith(FString Self, FString Value, 
#if UE_VERSION_NEWER_THAN(5, 6, -1)
		TOptional<EStringComparison> ComparisonType
#else
		EStringComparison ComparisonType = EStringComparison::Ordinal
#endif
	)
	{
		return Self.EndsWith(Value, ToSearchCase(ComparisonType));
	}

	UFUNCTION()
	static bool StartsWith(FString Self, FString Value,
#if UE_VERSION_NEWER_THAN(5, 6, -1)
		TOptional<EStringComparison> ComparisonType
#else
		EStringComparison ComparisonType = EStringComparison::Ordinal
#endif
	)
	{
		return Self.StartsWith(Value, ToSearchCase(ComparisonType));
	}

	UFUNCTION()
	static int IndexOf(FString Self, FString Value, 
#if UE_VERSION_NEWER_THAN(5, 6, -1)
		TOptional<EStringComparison> ComparisonType
#else
		EStringComparison ComparisonType = EStringComparison::Ordinal
#endif
	)
	{
		return Self.Find(*Value, ToSearchCase(ComparisonType), ESearchDir::FromStart);
	}

	UFUNCTION()
	static int LastIndexOf(FString Self, FString Value, 
#if UE_VERSION_NEWER_THAN(5, 6, -1)
		TOptional<EStringComparison> ComparisonType
#else
		EStringComparison ComparisonType = EStringComparison::Ordinal
#endif
	)
	{
		return Self.Find(*Value, ToSearchCase(ComparisonType), ESearchDir::FromEnd);
	}

	UFUNCTION()
	static FString Insert(FString Self, int32 StartIndex, FString Value)
	{
		return Self.Left(StartIndex) + Value + Self.RightChop(StartIndex);
	}

	UFUNCTION()
	static FString PadLeft(FString Self, int32 TotalWidth,
#if UE_VERSION_NEWER_THAN(5, 6, -1)
		TOptional<FString> PaddingChar
#else
		FString PaddingChar = TEXT(" ")
#endif
	)
	{
		const int32 PadCount = TotalWidth - Self.Len();
		if (PadCount <= 0) return Self;
#if UE_VERSION_NEWER_THAN(5, 6, -1)
		const TCHAR Ch = PaddingChar.IsSet() || PaddingChar.GetValue().IsEmpty() ? TEXT(' ') : PaddingChar.GetValue()[0];
#else
		const TCHAR Ch = PaddingChar.IsEmpty() ? TEXT(' ') : PaddingChar[0];
#endif
		
		return FString::ChrN(PadCount, Ch) + Self;
	}

	UFUNCTION()
	static FString PadRight(FString Self, int32 TotalWidth,
#if UE_VERSION_NEWER_THAN(5, 6, -1)
		TOptional<FString> PaddingChar
#else
		FString PaddingChar = TEXT(" ")
#endif
	)
	{
		const int32 PadCount = TotalWidth - Self.Len();
		if (PadCount <= 0) return Self;
#if UE_VERSION_NEWER_THAN(5, 6, -1)
		const TCHAR Ch = PaddingChar.IsSet() || PaddingChar.GetValue().IsEmpty() ? TEXT(' ') : PaddingChar.GetValue()[0];
#else
		const TCHAR Ch = PaddingChar.IsEmpty() ? TEXT(' ') : PaddingChar[0];
#endif
		return Self + FString::ChrN(PadCount, Ch);
	}


	UFUNCTION()
#if UE_VERSION_NEWER_THAN(5, 6, -1)
	static FString Remove(FString Self, int32 StartIndex, TOptional<int32> Count)
	{
		if (!Count.IsSet() || Count.GetValue() < 0) return Self.Left(StartIndex);
		return Self.Left(StartIndex) + Self.RightChop(StartIndex + Count.GetValue());
	}
#else
	static FString Remove(FString Self, int32 StartIndex, int32 Count = -1)
	{
		if (Count < 0) return Self.Left(StartIndex);
		return Self.Left(StartIndex) + Self.RightChop(StartIndex + Count);
	}
#endif

	UFUNCTION()
	static FString Replace(FString Self, FString OldValue, FString NewValue, 
#if UE_VERSION_NEWER_THAN(5, 6, -1)
		TOptional<EStringComparison> ComparisonType
#else
		EStringComparison ComparisonType = EStringComparison::Ordinal
#endif
	)
	{
		return Self.Replace(*OldValue, *NewValue, ToSearchCase(ComparisonType));
	}

	UFUNCTION()
	static FString ReplaceLineEndings(FString Self, FString ReplacementText = TEXT("\r\n"))
	{
		// Normalize to LF first, then replace with target to avoid double-replacing \r\n
		Self.ReplaceInline(TEXT("\r\n"), TEXT("\n"), ESearchCase::CaseSensitive);
		Self.ReplaceInline(TEXT("\r"),   TEXT("\n"), ESearchCase::CaseSensitive);
		Self.ReplaceInline(TEXT("\n"),   *ReplacementText, ESearchCase::CaseSensitive);
		return Self;
	}

	UFUNCTION()
	static TArray<FString> Split(FString Self, FString Separator, 
#if UE_VERSION_NEWER_THAN(5, 6, -1)
		TOptional<EStringSplitOptions> Options
#else
		EStringSplitOptions Options = EStringSplitOptions::None
#endif
	)
	{
#if UE_VERSION_NEWER_THAN(5, 6, -1)
		uint8 OptionFlags = static_cast<uint8>(Options.IsSet() ? Options.GetValue() : EStringSplitOptions::None);
#else
		uint8 OptionFlags = static_cast<uint8>(Options);
#endif
		
		
		TArray<FString> Result;
		const bool bCullEmpty = (OptionFlags & static_cast<uint8>(EStringSplitOptions::RemoveEmptyEntries)) != 0;
		Self.ParseIntoArray(Result, *Separator, bCullEmpty);
		if ((OptionFlags & static_cast<uint8>(EStringSplitOptions::TrimEntries)) != 0)
		{
			for (FString& Entry : Result)
			{
				Entry = Entry.TrimStartAndEnd();
			}
		}
		return Result;
	}

	UFUNCTION()
#if UE_VERSION_NEWER_THAN(5, 6, -1)
	static FString Substring(FString Self, int32 StartIndex, TOptional<int32> Length)
	{
		if (!Length.IsSet() || Length.GetValue() < 0) return Self.Mid(StartIndex);
		return Self.Mid(StartIndex, Length.GetValue());
	}
#else
	static FString Substring(FString Self, int32 StartIndex, int32 Length = -1)
	{
		if (Length < 0) return Self.Mid(StartIndex);
		return Self.Mid(StartIndex, Length);
	}
#endif

	UFUNCTION()
	static FString ToLower(FString Self)
	{
		return Self.ToLower();
	}

	UFUNCTION()
	static FString ToLowerInvariant(FString Self)
	{
		return Self.ToLower();
	}

	UFUNCTION()
	static FString ToUpper(FString Self)
	{
		return Self.ToUpper();
	}

	UFUNCTION()
	static FString ToUpperInvariant(FString Self)
	{
		return Self.ToUpper();
	}

	UFUNCTION()
	static FString Trim(FString Self, 
#if UE_VERSION_NEWER_THAN(5, 6, -1)
	TOptional<FString> Chars
#else
	FString Chars = TEXT("")
#endif	
	)
	{
#if UE_VERSION_NEWER_THAN(5, 6, -1)
		FString CharsToTrim = Chars.IsSet() ? Chars.GetValue() : TEXT("");
#else
		FString CharsToTrim = Chars;
#endif
		if (CharsToTrim.IsEmpty()) return Self.TrimStartAndEnd();
		int32 Start = 0;
		while (Start < Self.Len() && CharsToTrim.GetCharArray().Contains(Self[Start])) Start++;
		int32 End = Self.Len() - 1;
		while (End >= Start && CharsToTrim.GetCharArray().Contains(Self[End])) End--;
		return Self.Mid(Start, End - Start + 1);
	}

	UFUNCTION()
	static FString TrimEnd(FString Self,
#if UE_VERSION_NEWER_THAN(5, 6, -1)
	TOptional<FString> Chars
#else
	FString Chars = TEXT("")
#endif	
	)
	{
#if UE_VERSION_NEWER_THAN(5, 6, -1)
		FString CharsToTrim = Chars.IsSet() ? Chars.GetValue() : TEXT("");
#else
		FString CharsToTrim = Chars;
#endif
		if (CharsToTrim.IsEmpty()) return Self.TrimEnd();
		int32 End = Self.Len() - 1;
		while (End >= 0 && CharsToTrim.GetCharArray().Contains(Self[End])) End--;
		return Self.Left(End + 1);
	}

	UFUNCTION()
	static FString TrimStart(FString Self,
#if UE_VERSION_NEWER_THAN(5, 6, -1)
	TOptional<FString> Chars
#else
	FString Chars = TEXT("")
#endif	
	)
	{
#if UE_VERSION_NEWER_THAN(5, 6, -1)
		FString CharsToTrim = Chars.IsSet() ? Chars.GetValue() : TEXT("");
#else
		FString CharsToTrim = Chars;
#endif
		if (CharsToTrim.IsEmpty()) return Self.TrimStart();
		int32 Start = 0;
		while (Start < Self.Len() && CharsToTrim.GetCharArray().Contains(Self[Start])) Start++;
		return Self.RightChop(Start);
	}

	/* static methods */

	UFUNCTION()
	static bool Equals(FString A, FString B, 
#if UE_VERSION_NEWER_THAN(5, 6, -1)
		TOptional<EStringComparison> ComparisonType
#else
		EStringComparison ComparisonType = EStringComparison::Ordinal
#endif
	)
	{
		return A.Equals(B, ToSearchCase(ComparisonType));
	}

	UFUNCTION()
	static int Compare(FString StrA, FString StrB, 
#if UE_VERSION_NEWER_THAN(5, 6, -1)
		TOptional<EStringComparison> ComparisonType
#else
		EStringComparison ComparisonType = EStringComparison::Ordinal
#endif
	)
	{
		if (ToSearchCase(ComparisonType) == ESearchCase::IgnoreCase)
			return FCString::Stricmp(*StrA, *StrB);
		return FCString::Strcmp(*StrA, *StrB);
	}

	UFUNCTION()
	static int CompareOrdinal(FString StrA, FString StrB)
	{
		return FCString::Strcmp(*StrA, *StrB);
	}

	UFUNCTION()
	static bool IsNullOrEmpty(FString Value)
	{
		return Value.IsEmpty();
	}

	UFUNCTION()
	static bool IsNullOrWhiteSpace(FString Value)
	{
		return Value.TrimStartAndEnd().IsEmpty();
	}

};
