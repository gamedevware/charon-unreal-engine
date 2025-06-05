// Copyright GameDevWare, Denis Zykov 2025

#include "GameData/Formatters/FMessagePackGameDataReader.h"
#include "Misc/Base64.h"

enum class EMessagePackType : uint8
{
	PositiveFixIntStart = 0x00,
	PositiveFixIntEnd = 0x7f,
	FixMapStart = 0x80,
	FixMapEnd = 0x8f,
	FixArrayStart = 0x90,
	FixArrayEnd = 0x9f,
	FixStrStart = 0xa0,
	FixStrEnd = 0xbf,
	Null = 0xc0,
	Unused = 0xc1,
	False = 0xc2,
	True = 0xc3,
	Bin8 = 0xc4,
	Bin16 = 0xc5,
	Bin32 = 0xc6,
	Ext8 = 0xc7,
	Ext16 = 0xc8,
	Ext32 = 0xc9,
	Float32 = 0xca,
	Float64 = 0xcb,
	UInt8 = 0xcc,
	UInt16 = 0xcd,
	UInt32 = 0xce,
	UInt64 = 0xcf,
	Int8 = 0xd0,
	Int16 = 0xd1,
	Int32 = 0xd2,
	Int64 = 0xd3,
	FixExt1 = 0xd4,
	FixExt2 = 0xd5,
	FixExt4 = 0xd6,
	FixExt8 = 0xd7,
	FixExt16 = 0xd8,
	Str8 = 0xd9,
	Str16 = 0xda,
	Str32 = 0xdb,
	Array16 = 0xdc,
	Array32 = 0xdd,
	Map16 = 0xde,
	Map32 = 0xdf,
	NegativeFixIntStart = 0xe0,
	NegativeFixIntEnd = 0xff
};

enum EMessagePackParseMemberState : uint8
{
	EmptyStack = 0,
	NotAMap = 1,
	MemberNameExpected = 2,
	ValueExpected = 3,
	EndOfMapExpected = 4
};

void FMessagePackGameDataReader::DecrementClosingTokenCounter()
{
	if (ParseState.Num() <= 0)
	{
		return;
	}
	const auto TopToken = ParseState.Pop();
	PushClosingTokenCounter(TopToken.Key, TopToken.Value - 1);
}

void FMessagePackGameDataReader::PushClosingTokenCounter(EJsonNotation ClosingNotation, int32 Size)
{
	const auto NewState = TTuple<EJsonNotation, int32>(ClosingNotation, Size);
	ParseState.Push(NewState);
}

void FMessagePackGameDataReader::ReadBytes(TArray<uint8>& Buffer, const int32 NumBytes) const
{
	Buffer.SetNumUninitialized(NumBytes);
	for (int32 Index = 0; Index < NumBytes; ++Index)
	{
		(*Stream) << Buffer[Index];
	}
}

FMessagePackGameDataReader::FMessagePackGameDataReader(FArchive* const Stream)
	: Stream(Stream)
	  , ParseState()
	  , Notation(EJsonNotation::Null)
	  , CurrentToken(EJsonToken::None)
	  , Identifier()
	  , ErrorMessage()
	  , StringValue()
	  , NumberValue(0.0f)
	  , BoolValue(false)
	  , FinishedReadingRootObject(false)
{
}

bool FMessagePackGameDataReader::ReadNext()
{
	if (!ErrorMessage.IsEmpty())
	{
		Notation = EJsonNotation::Error;
		CurrentToken = EJsonToken::None;
		return false;
	}

	if (Stream == nullptr)
	{
		Notation = EJsonNotation::Error;
		CurrentToken = EJsonToken::None;
		SetErrorMessage(TEXT("Null Stream"));
		return true;
	}

	// reset state
	Notation = EJsonNotation::Null;
	CurrentToken = EJsonToken::None;
	ErrorMessage.Empty();
	StringValue.Empty();
	NumberValue = 0;
	BoolValue = false;
	//
		
	if (ParseState.Num() > 0 && ParseState.Top().Value == 0)
	{
		Notation = ParseState.Pop().Key;
		switch (Notation)
		{
		case EJsonNotation::ObjectEnd: CurrentToken = EJsonToken::CurlyClose; break;
		case EJsonNotation::ArrayEnd: CurrentToken = EJsonToken::SquareClose; break;
		default:
			Notation = EJsonNotation::Error;
			CurrentToken = EJsonToken::None;
			SetErrorMessage(TEXT("Unexpected closing notation for object/array."));
			break;
		}
		DecrementClosingTokenCounter();

		if (ParseState.Num() == 0)
		{
			FinishedReadingRootObject = true;
		}

		return true;
	}

	const bool AtEndOfStream = Stream->AtEnd();
	if (AtEndOfStream && !FinishedReadingRootObject)
	{
		Notation = EJsonNotation::Error;
		CurrentToken = EJsonToken::None;
		SetErrorMessage(TEXT("Improperly formatted."));
		return true;
	}

	if (!AtEndOfStream && FinishedReadingRootObject)
	{
		Notation = EJsonNotation::Error;
		CurrentToken = EJsonToken::None;
		SetErrorMessage(TEXT("Unexpected additional input found."));
		return true;
	}

	if (AtEndOfStream)
	{
		return false;
	}

	if (PeekParseMapState() != EMessagePackParseMemberState::ValueExpected)
	{
		Identifier.Empty();
	}

	uint8 FormatValue = ReadUInt8();
	if (FormatValue >= static_cast<uint8>(EMessagePackType::FixArrayStart) && FormatValue <= static_cast<uint8>(EMessagePackType::FixArrayEnd))
	{
		const auto ArraySize = static_cast<int>(FormatValue) - static_cast<int>(EMessagePackType::FixArrayStart);
		PushClosingTokenCounter(EJsonNotation::ArrayEnd, ArraySize + 1);
		Notation = EJsonNotation::ArrayStart;
		CurrentToken = EJsonToken::SquareOpen;
	}
	else if (FormatValue >= static_cast<uint8>(EMessagePackType::FixStrStart) && FormatValue <= static_cast<uint8>(EMessagePackType::FixStrEnd))
	{
		const auto Utf8StringSize = static_cast<int>(FormatValue) - static_cast<int>(EMessagePackType::FixStrStart);
		TArray<uint8> Utf8StringBytes;
		ReadBytes(Utf8StringBytes, Utf8StringSize);
		Utf8StringBytes.Add(0);
		
		auto Utf8String = FString(UTF8_TO_TCHAR(Utf8StringBytes.GetData()));

		if (PeekParseMapState() == EMessagePackParseMemberState::MemberNameExpected)
		{
			DecrementClosingTokenCounter();

			Identifier = Utf8String;

			return ReadNext();
		}

		StringValue = Utf8String;
		Notation = EJsonNotation::String;
		CurrentToken = EJsonToken::String;
	}
	else if (FormatValue >= static_cast<uint8>(EMessagePackType::FixMapStart) && FormatValue <= static_cast<uint8>(EMessagePackType::FixMapEnd))
	{
		const auto MapSize = static_cast<int>(FormatValue) - static_cast<int>(EMessagePackType::FixMapStart);

		PushClosingTokenCounter(EJsonNotation::ObjectEnd, MapSize * 2 + 1);
		Notation = EJsonNotation::ObjectStart;
		CurrentToken = EJsonToken::CurlyOpen;
	}
	else if (FormatValue >= static_cast<uint8>(EMessagePackType::NegativeFixIntStart))
	{
		NumberValue = *reinterpret_cast<int8*>(&FormatValue);
		Notation = EJsonNotation::Number;
		CurrentToken = EJsonToken::Number;
	}
	else if (FormatValue <= static_cast<uint8>(EMessagePackType::PositiveFixIntEnd))
	{
		NumberValue = FormatValue;
		Notation = EJsonNotation::Number;
		CurrentToken = EJsonToken::Number;
	}
	else
	{
		switch (static_cast<EMessagePackType>(FormatValue))
		{
		case EMessagePackType::Null:
			Notation = EJsonNotation::Null;
			CurrentToken = EJsonToken::Null;
			break;
		case EMessagePackType::Array16:
		case EMessagePackType::Array32:
			{
				uint32 ArraySize = 0;
				if (FormatValue == static_cast<uint8>(EMessagePackType::Array16))
				{
					ArraySize = ReadBeUInt16();
				}
				else if (FormatValue == static_cast<uint8>(EMessagePackType::Array32))
				{
					ArraySize = ReadBeUInt32();
				}

				PushClosingTokenCounter(EJsonNotation::ArrayEnd, ArraySize + 1);

				Notation = EJsonNotation::ArrayStart;
				CurrentToken = EJsonToken::SquareOpen;
				break;
			}
		case EMessagePackType::Map16:
		case EMessagePackType::Map32:
			{
				uint32 MapSize = 0;
				if (FormatValue == static_cast<uint8>(EMessagePackType::Map16))
				{
					MapSize = ReadBeUInt16();
				}
				else if (FormatValue == static_cast<uint8>(EMessagePackType::Map32))
				{
					MapSize = ReadBeUInt32();
				}

				PushClosingTokenCounter(EJsonNotation::ObjectEnd, MapSize * 2 + 1);

				Notation = EJsonNotation::ObjectStart;
				CurrentToken = EJsonToken::CurlyOpen;
				break;
			}
		case EMessagePackType::Str16:
		case EMessagePackType::Str32:
		case EMessagePackType::Str8:
			{
				auto Utf8StringSize = 0L;
				if (FormatValue == static_cast<uint8>(EMessagePackType::Str8))
				{
					Utf8StringSize = ReadUInt8();
				}
				else if (FormatValue == static_cast<uint8>(EMessagePackType::Str16))
				{
					Utf8StringSize = ReadBeUInt16();
				}
				else if (FormatValue == static_cast<uint8>(EMessagePackType::Str32))
				{
					Utf8StringSize = ReadBeUInt32();
				}

				if (Utf8StringSize > MAX_BINARY_LENGTH)
				{
					Notation = EJsonNotation::Error;
					CurrentToken = EJsonToken::None;
					SetErrorMessage(TEXT("Binary data is too long."));
					return true;
				}

				TArray<uint8> Utf8StringBytes;
				ReadBytes(Utf8StringBytes, Utf8StringSize);
				Utf8StringBytes.Add(0);
				auto Utf8String = FString(UTF8_TO_TCHAR(Utf8StringBytes.GetData()));

				if (PeekParseMapState() == EMessagePackParseMemberState::MemberNameExpected)
				{
					DecrementClosingTokenCounter();

					Identifier = Utf8String;

					return ReadNext();
				}

				StringValue = Utf8String;
				Notation = EJsonNotation::String;
				CurrentToken = EJsonToken::String;
			}
		case EMessagePackType::Bin32:
		case EMessagePackType::Bin16:
		case EMessagePackType::Bin8:
			{
				auto BinarySize = 0L;
				if (FormatValue == static_cast<uint8>(EMessagePackType::Bin8))
				{
					BinarySize = ReadUInt8();
				}
				else if (FormatValue == static_cast<uint8>(EMessagePackType::Bin16))
				{
					BinarySize = ReadBeUInt16();
				}
				else if (FormatValue == static_cast<uint8>(EMessagePackType::Bin32))
				{
					BinarySize = ReadBeUInt32();
				}

				if (BinarySize > MAX_BINARY_LENGTH)
				{
					Notation = EJsonNotation::Error;
					CurrentToken = EJsonToken::None;
					SetErrorMessage(TEXT("Binary data is too long."));
					return true;
				}

				TArray<uint8> BinaryBytes;
				ReadBytes(BinaryBytes, BinarySize);
				auto BinaryBase64 = FBase64::Encode(BinaryBytes);

				StringValue = BinaryBase64;
				Notation = EJsonNotation::String;
				CurrentToken = EJsonToken::String;
				break;
			}
		case EMessagePackType::FixExt1:
		case EMessagePackType::FixExt16:
		case EMessagePackType::FixExt2:
		case EMessagePackType::FixExt4:
		case EMessagePackType::FixExt8:
		case EMessagePackType::Ext32:
		case EMessagePackType::Ext16:
		case EMessagePackType::Ext8:
			{
				uint32 ExtSize = 0;
				if (FormatValue == static_cast<uint8>(EMessagePackType::FixExt1))
				{
					ExtSize = 1;
				}
				else if (FormatValue == static_cast<uint8>(EMessagePackType::FixExt2))
				{
					ExtSize = 2;
				}
				else if (FormatValue == static_cast<uint8>(EMessagePackType::FixExt4))
				{
					ExtSize = 4;
				}
				else if (FormatValue == static_cast<uint8>(EMessagePackType::FixExt8))
				{
					ExtSize = 8;
				}
				else if (FormatValue == static_cast<uint8>(EMessagePackType::FixExt16))
				{
					ExtSize = 16;
				}
				if (FormatValue == static_cast<uint8>(EMessagePackType::Ext8))
				{
					ExtSize = ReadUInt8();
				}
				else if (FormatValue == static_cast<uint8>(EMessagePackType::Ext16))
				{
					ExtSize = ReadBeUInt16();
				}
				else if (FormatValue == static_cast<uint8>(EMessagePackType::Ext32))
				{
					ExtSize = ReadBeUInt32();
				}

				if (ExtSize > MAX_BINARY_LENGTH)
				{
					Notation = EJsonNotation::Error;
					CurrentToken = EJsonToken::None;
					SetErrorMessage(TEXT("Extension data is too long."));
					return true;
				}

				TArray<uint8> ExtBytes;
				ReadBytes(ExtBytes, ExtSize);
				auto ExtBase64 = FBase64::Encode(ExtBytes);

				StringValue = ExtBase64;
				Notation = EJsonNotation::String;
				CurrentToken = EJsonToken::String;
				break;
			}
		case EMessagePackType::False:
			BoolValue = false;
			Notation = EJsonNotation::Boolean;
			CurrentToken = EJsonToken::False;
			break;
		case EMessagePackType::True:
			BoolValue = true;
			Notation = EJsonNotation::Boolean;
			CurrentToken = EJsonToken::True;
			break;
		case EMessagePackType::Float32:
			NumberValue = ReadBeFloat32();
			Notation = EJsonNotation::Number;
			CurrentToken = EJsonToken::Number;
			break;
		case EMessagePackType::Float64:
			NumberValue = ReadFloat64();
			Notation = EJsonNotation::Number;
			CurrentToken = EJsonToken::Number;
			break;
		case EMessagePackType::Int8:
			NumberValue = ReadInt8();
			Notation = EJsonNotation::Number;
			CurrentToken = EJsonToken::Number;
			break;
		case EMessagePackType::Int16:
			{
				NumberValue = ReadBeInt16();
				Notation = EJsonNotation::Number;
				CurrentToken = EJsonToken::Number;
				break;
			}
		case EMessagePackType::Int32:
			{
				NumberValue = ReadBeInt32();
				Notation = EJsonNotation::Number;
				CurrentToken = EJsonToken::Number;
				break;
			}
		case EMessagePackType::Int64:
			{
				int64 Int64Value = ReadBeInt64();
				if (Int64Value >= INT32_MAX || Int64Value <= INT32_MIN)
				{
					StringValue = FString::Printf(TEXT("%lld"), Int64Value);
					Notation = EJsonNotation::String;
					CurrentToken = EJsonToken::String;
				}
				else
				{
					NumberValue = Int64Value;
					Notation = EJsonNotation::Number;
					CurrentToken = EJsonToken::Number;
				}
				break;
			}
		case EMessagePackType::UInt8:
			{
				NumberValue = ReadUInt8();
				Notation = EJsonNotation::Number;
				CurrentToken = EJsonToken::Number;
				break;
			}
		case EMessagePackType::UInt16:
			{
				NumberValue = ReadBeUInt16();
				Notation = EJsonNotation::Number;
				CurrentToken = EJsonToken::Number;
				break;
			}
		case EMessagePackType::UInt32:
			{
				NumberValue = ReadBeUInt32();
				Notation = EJsonNotation::Number;
				CurrentToken = EJsonToken::Number;
				break;
			}
		case EMessagePackType::UInt64:
			{
				uint64 UInt64Value = ReadBeUInt64();
				if (UInt64Value >= INT32_MAX)
				{
					StringValue = FString::Format(TEXT("{0}"), { UInt64Value });
					Notation = EJsonNotation::String;
					CurrentToken = EJsonToken::String;
				}
				else
				{
					NumberValue = UInt64Value;
					Notation = EJsonNotation::Number;
					CurrentToken = EJsonToken::Number;
				}
				break;
			}
		case EMessagePackType::PositiveFixIntStart:
		case EMessagePackType::PositiveFixIntEnd:
		case EMessagePackType::FixMapStart:
		case EMessagePackType::FixMapEnd:
		case EMessagePackType::FixArrayStart:
		case EMessagePackType::FixArrayEnd:
		case EMessagePackType::FixStrStart:
		case EMessagePackType::FixStrEnd:
		case EMessagePackType::Unused:
		case EMessagePackType::NegativeFixIntStart:
		case EMessagePackType::NegativeFixIntEnd:
		default:
			Notation = EJsonNotation::Error;
			CurrentToken = EJsonToken::None;
			SetErrorMessage(TEXT("Unexpected Message Pack notation."));
			return true;
		}
	}

	DecrementClosingTokenCounter();

	return true;
}

const FString& FMessagePackGameDataReader::GetIdentifier() const
{
	return Identifier;
}

const FString FMessagePackGameDataReader::GetValueAsString() const
{
	check(CurrentToken == EJsonToken::String);

	return StringValue;
}

double FMessagePackGameDataReader::GetValueAsNumber() const
{
	check(CurrentToken == EJsonToken::Number);

	return NumberValue;
}

bool FMessagePackGameDataReader::GetValueAsBoolean() const
{
	check((CurrentToken == EJsonToken::True) || (CurrentToken == EJsonToken::False));

	return BoolValue;
}

const FString& FMessagePackGameDataReader::GetErrorMessage() const
{
	return ErrorMessage;
}

EJsonNotation FMessagePackGameDataReader::GetNotation()
{
	return Notation;
}

void FMessagePackGameDataReader::SetErrorState(const FString& Message)
{
	ErrorMessage = Message;
	Notation = EJsonNotation::Error;
}

void FMessagePackGameDataReader::SetErrorMessage(const FString& Message)
{
	ErrorMessage = Message;
}

uint8 FMessagePackGameDataReader::PeekParseMapState()
{
	if (ParseState.Num() == 0)
	{
		return EMessagePackParseMemberState::EmptyStack;
	}
	const auto ClosingToken = ParseState.Top();
	if (ClosingToken.Key != EJsonNotation::ObjectEnd)
	{
		return EMessagePackParseMemberState::NotAMap;
	}
	else if (ClosingToken.Value == 0)
	{
		return EMessagePackParseMemberState::EndOfMapExpected;
	}
	else if ((ClosingToken.Value % 2) == 0)
	{
		return EMessagePackParseMemberState::MemberNameExpected;
	}
	else
	{
		return EMessagePackParseMemberState::ValueExpected;
	}
}

int8 FMessagePackGameDataReader::ReadInt8() const
{
	int8 leValue = 0;
	Stream->Serialize(&leValue, 1);
	return leValue;
}

uint8 FMessagePackGameDataReader::ReadUInt8() const
{
	uint8 leValue = 0;
	Stream->Serialize(&leValue, 1);
	return leValue;
}

int16 FMessagePackGameDataReader::ReadBeInt16() const
{
	uint16 beValue = this->ReadBeUInt16();
	return *reinterpret_cast<int16*>(&beValue);
}

uint16 FMessagePackGameDataReader::ReadBeUInt16() const
{
	uint16 leValue = 0;
	Stream->Serialize(&leValue, 2);
	return (leValue << 8) | (leValue >> 8);
}

int32 FMessagePackGameDataReader::ReadBeInt32() const
{
	uint32 beValue = this->ReadBeUInt32();
	return *reinterpret_cast<int32*>(&beValue);
}

uint32 FMessagePackGameDataReader::ReadBeUInt32() const
{
	uint32 leValue = 0;
	Stream->Serialize(&leValue, 4);
	return ((leValue >> 24) & 0x000000FF) |
		   ((leValue >>  8) & 0x0000FF00) |
		   ((leValue <<  8) & 0x00FF0000) |
		   ((leValue << 24) & 0xFF000000);
}

int64 FMessagePackGameDataReader::ReadBeInt64() const
{
	uint64 beValue = this->ReadBeUInt64();
	return *reinterpret_cast<int64*>(&beValue);
}

uint64 FMessagePackGameDataReader::ReadBeUInt64() const
{
	uint64 leValue = 0;
	Stream->Serialize(&leValue, 8);
	return ((leValue >> 56) & 0x00000000000000FF) |
		   ((leValue >> 40) & 0x000000000000FF00) |
		   ((leValue >> 24) & 0x0000000000FF0000) |
		   ((leValue >>  8) & 0x00000000FF000000) |
		   ((leValue <<  8) & 0x000000FF00000000) |
		   ((leValue << 24) & 0x0000FF0000000000) |
		   ((leValue << 40) & 0x00FF000000000000) |
		   ((leValue << 56) & 0xFF00000000000000);
}

float FMessagePackGameDataReader::ReadBeFloat32() const
{
	uint32 beValue = ReadBeUInt32();
	return *reinterpret_cast<float*>(&beValue);
}

double FMessagePackGameDataReader::ReadFloat64() const
{
	uint64 beValue = ReadBeUInt64();
    return *reinterpret_cast<double*>(&beValue);
}
