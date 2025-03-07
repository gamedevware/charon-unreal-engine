// Copyright GameDevWare, Denis Zykov 2024

#if WITH_TESTS

#include "Tests/TestHarnessAdapter.h"
#include "Misc/Base64.h"
#include "GameData/Formatters/FMessagePackGameDataReader.h"

TEST_CASE_NAMED(FMessagePackGameDataReaderTest, "Charon::FMessagePackGameDataReaderTest", "[Core]")
{
	/*
		string ToMsgPackBase64(string json) {
			var stream = new System.IO.MemoryStream();
			GameDevWare.Serialization.MsgPack.Serialize(GameDevWare.Serialization.Json.Deserialize(typeof(object), new StringReader(json)), stream, GameDevWare.Serialization.SerializationOptions.SuppressTypeInformation);
			return Convert.ToBase64String(stream.ToArray());
		}

		string ToMsgPackBase64Obj(object value) {
			var stream = new System.IO.MemoryStream();
			GameDevWare.Serialization.MsgPack.Serialize(value, stream, GameDevWare.Serialization.SerializationOptions.SuppressTypeInformation);
			return Convert.ToBase64String(stream.ToArray());
		}
	 */

	
	SECTION("ReadTokens")
	{
		TArray<uint8> Buffer;
		auto Base64Chars = FString(TEXT("gaZ2YWx1ZXOWpHRleHQBw8LAyj+MzM0="));
		FBase64::Decode(Base64Chars, Buffer, EBase64Mode::Standard);
		auto Stream = MakeUnique<FBufferReader>(Buffer.GetData(), Buffer.Num(), false, false);

		auto ExpectedTokens = TArray<EJsonNotation>({
			EJsonNotation::ObjectStart, EJsonNotation::ArrayStart, EJsonNotation::String, EJsonNotation::Number,
			EJsonNotation::Boolean, EJsonNotation::Boolean, EJsonNotation::Null, EJsonNotation::Number, EJsonNotation::ArrayEnd, EJsonNotation::ObjectEnd
		});
		auto ActualTokens = TArray<EJsonNotation>();

		auto Reader = MakeUnique<FMessagePackGameDataReader>(Stream.Get());
		while (Reader->ReadNext())
		{
			ActualTokens.Add(Reader->GetNotation());
		}

		REQUIRE(ExpectedTokens.Num() == ActualTokens.Num());
		for (int i = 0; i < ExpectedTokens.Num(); ++i)
		{
			CHECK(ExpectedTokens[i] == ActualTokens[i]);
		}
	}

	SECTION("ReadValues")
	{
		TArray<uint8> Buffer;
		auto Base64Chars = FString(TEXT("gaZ2YWx1ZXOWpHRleHQBw8LAyj/AAAA="));
		FBase64::Decode(Base64Chars, Buffer, EBase64Mode::Standard);
		auto Stream = MakeUnique<FBufferReader>(Buffer.GetData(), Buffer.Num(), false, false);

		auto ExpectedValues = TArray<FString>({
			"", "", "text", "1.000000", "true", "false", "null", "1.500000", "", ""
		});
		auto ActualValues = TArray<FString>();

		auto Reader = MakeUnique<FMessagePackGameDataReader>(Stream.Get());
		while (Reader->ReadNext())
		{
			switch (Reader->GetNotation())
			{
			case EJsonNotation::String:
                ActualValues.Add(Reader->GetValueAsString());
                break;
			case EJsonNotation::Number:
				ActualValues.Add(FString::Format(TEXT("{0}"), { Reader->GetValueAsNumber() }));
				break;
			case EJsonNotation::Null:
				ActualValues.Add(TEXT("null"));
				break;
			case EJsonNotation::Boolean:
				ActualValues.Add(Reader->GetValueAsBoolean() ? TEXT("true"): TEXT("false"));
				break;
			default:
				ActualValues.Add(TEXT(""));
				break;
			}
		}

		REQUIRE(ExpectedValues.Num() == ActualValues.Num());
		for (int i = 0; i < ExpectedValues.Num(); ++i)
		{
			CHECK_MESSAGE(FString::Format(TEXT("Expected: {0}, Actual: {1}"), { ExpectedValues[i], ActualValues[i] }), ExpectedValues[i] == ActualValues[i]);
		}
	}

	SECTION("ReadNumbers")
	{
		TArray<uint8> Buffer;
		auto Base64Chars = FString(TEXT("jqJ2MdEA/6J2MtCAonYzf6J2NNIAAP//onY10YAAonY20X//onY3zv////+idjjSgAAAAKJ2OdJ/////onoxz///////////onoy04AAAAAAAAAAonoz03//////////ono0yn9///+iejXLf+////////8="));
		FBase64::Decode(Base64Chars, Buffer, EBase64Mode::Standard);
		auto Stream = MakeUnique<FBufferReader>(Buffer.GetData(), Buffer.Num(), false, false);

		auto ExpectedValues = TArray<FString>({
			"", "255", "-128", "127", "65535", "-32768", "32767", "4294967295", "-2147483648", "2147483647", "18446744073709551615", "-9223372036854775808", "9223372036854775807",
			"340282346638528859811704183484516925440.000000", "179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.000000", ""
		});
		auto ActualValues = TArray<FString>();

		auto Reader = MakeUnique<FMessagePackGameDataReader>(Stream.Get());
		while (Reader->ReadNext())
		{
			switch (Reader->GetNotation())
			{
			case EJsonNotation::String:
				ActualValues.Add(Reader->GetValueAsString());
				break;
			case EJsonNotation::Number:
				{
					double Number = Reader->GetValueAsNumber();
					if (static_cast<double>(static_cast<int64>(Number)) == Number)
					{
						auto Integer = static_cast<int64>(Number);
						ActualValues.Add(FString::Format(TEXT("{0}"), { Integer }));
					}
					else
					{
						ActualValues.Add(FString::Format(TEXT("{0}"), { Number }));
					}
					break;
				}
			case EJsonNotation::Null:
				ActualValues.Add(TEXT("null"));
				break;
			case EJsonNotation::Boolean:
				ActualValues.Add(Reader->GetValueAsBoolean() ? TEXT("true"): TEXT("false"));
				break;
			default:
				ActualValues.Add(TEXT(""));
				break;
			}
		}

		REQUIRE(ExpectedValues.Num() == ActualValues.Num());
		for (int i = 0; i < ExpectedValues.Num(); ++i)
		{
			CHECK_MESSAGE(FString::Format(TEXT("Expected: {0}, Actual: {1}"), { ExpectedValues[i], ActualValues[i] }), ExpectedValues[i] == ActualValues[i]);
		}
	}

	SECTION("StringSimpleEscapes")
	{
		TArray<uint8> Buffer;
		auto Base64Chars = FString(TEXT("kapcICIvCAwJIg0K"));
		FBase64::Decode(Base64Chars, Buffer, EBase64Mode::Standard);
		auto Stream = MakeUnique<FBufferReader>(Buffer.GetData(), Buffer.Num(), false, false);

		auto ExpectedValues = TArray<FString>({
			"", TEXT("\\ \"/\x08\x0C\t\"\r\n"), ""
		});
		auto ActualValues = TArray<FString>();

		auto Reader = MakeUnique<FMessagePackGameDataReader>(Stream.Get());
		while (Reader->ReadNext())
		{
			switch (Reader->GetNotation())
			{
			case EJsonNotation::String:
				ActualValues.Add(Reader->GetValueAsString());
				break;
			default:
				ActualValues.Add(TEXT(""));
				break;
			}
		}

		REQUIRE(ExpectedValues.Num() == ActualValues.Num());
		for (int i = 0; i < ExpectedValues.Num(); ++i)
		{
			CHECK_MESSAGE(FString::Format(TEXT("Expected: {0}, Actual: {1}"), { ExpectedValues[i], ActualValues[i] }), ExpectedValues[i] == ActualValues[i]);
		}
	}

	SECTION("StringUnicodeEscapes")
	{
		TArray<uint8> Buffer;
		auto Base64Chars = FString(TEXT("kaPkuJY="));
		FBase64::Decode(Base64Chars, Buffer, EBase64Mode::Standard);
		auto Stream = MakeUnique<FBufferReader>(Buffer.GetData(), Buffer.Num(), false, false);

		auto ExpectedValues = TArray<FString>({
			"", TEXT("\u4E16"), ""
		});
		auto ActualValues = TArray<FString>();

		auto Reader = MakeUnique<FMessagePackGameDataReader>(Stream.Get());
		while (Reader->ReadNext())
		{
			switch (Reader->GetNotation())
			{
			case EJsonNotation::String:
				ActualValues.Add(Reader->GetValueAsString());
				break;
			default:
				ActualValues.Add(TEXT(""));
				break;
			}
		}

		REQUIRE(ExpectedValues.Num() == ActualValues.Num());
		for (int i = 0; i < ExpectedValues.Num(); ++i)
		{
			CHECK(ExpectedValues[i] == ActualValues[i]);
		}
	}
}

#endif //WITH_TESTS