// Copyright GameDevWare, Denis Zykov 2025

// ReSharper disable CppUseFamiliarTemplateSyntaxForGenericLambdas
#include "FDotNetStringType.h"
#include "UObject/UnrealType.h"

void FDotNetStringType::InitializeStaticFunctions(UClass* DeclaringClass,
	const TSharedRef<TMap<FString, FFormulaFunction>> FunctionList)
{
	FDotNetSurrogateType::InitializeStaticFunctions(DeclaringClass, FunctionList);

	UField* const DeclaringType = GetTypeClassOrStruct();
	FunctionList->Add(TEXT("Concat"),  GetConcatFunction(DeclaringType));
	FunctionList->Add(TEXT("Format"),  GetFormatFunction(DeclaringType));
	FunctionList->Add(TEXT("Join"),    GetJoinFunction(DeclaringType));
}

void FDotNetStringType::InitializeStaticFunctionNames(UClass* DeclaringClass, const TSharedRef<TArray<FString>> NameList)
{
	FDotNetSurrogateType::InitializeStaticFunctionNames(DeclaringClass, NameList);

	NameList->Add(TEXT("Concat"));
	NameList->Add(TEXT("Format"));
	NameList->Add(TEXT("Join"));
}

FFormulaFunction FDotNetStringType::GetConcatFunction(UField* DeclaringType)
{
	return FFormulaFunction([](
		const TSharedRef<FFormulaValue>&,
		const FFormulaInvokeArguments& Arguments,
		const UField*,
		const TArray<UField*>*, TSharedPtr<FFormulaValue>& Result)
		{
			FString ResultString;
			int32 ArgumentIndex = 0;
			FString ArgumentName;
			ArgumentName.Append("Arg");
			ArgumentName.AppendInt(ArgumentIndex);

			while (const auto Argument = Arguments.FindArgument(ArgumentName, ArgumentIndex))
			{
				const auto ArgumentValue = Argument->UpdatedValue.IsValid()
					? Argument->UpdatedValue
					: Argument->Value;

				ArgumentValue->WriteText(ResultString);

				ArgumentIndex++;
				ArgumentName.Reset();
				ArgumentName.Append("Arg");
				ArgumentName.AppendInt(ArgumentIndex);
			}
			Result = MakeShared<FFormulaValue>(ResultString);
			return true;
		},
		TArray<const FProperty*>(),
		DeclaringType,
		/*bUseClassDefaultObject*/ true);
}

FFormulaFunction FDotNetStringType::GetFormatFunction(UField* DeclaringType)
{
	return FFormulaFunction([](
		const TSharedRef<FFormulaValue>&,
		const FFormulaInvokeArguments& Arguments,
		const UField*,
		const TArray<UField*>*, TSharedPtr<FFormulaValue>& Result)
		{
			// Arg "Format" at index 0 is the format string
			const auto FormatArg = Arguments.FindArgument(TEXT("Format"), 0);
			if (!FormatArg)
			{
				return false;
			}
			const auto FormatValue = FormatArg->UpdatedValue.IsValid()
				? FormatArg->UpdatedValue
				: FormatArg->Value;
			FString FormatStr;
			FormatValue->WriteText(FormatStr);

			// Collect "Arg0", "Arg1", ... starting at index 1
			TArray<FString> Args;
			int32 ArgIdx = 0;
			FString ArgName;
			ArgName.Append(TEXT("Arg"));
			ArgName.AppendInt(ArgIdx);
			while (const auto Arg = Arguments.FindArgument(ArgName, ArgIdx + 1))
			{
				const auto ArgValue = Arg->UpdatedValue.IsValid() ? Arg->UpdatedValue : Arg->Value;
				FString ArgStr;
				ArgValue->WriteText(ArgStr);
				Args.Add(ArgStr);
				ArgIdx++;
				ArgName.Reset();
				ArgName.Append(TEXT("Arg"));
				ArgName.AppendInt(ArgIdx);
			}

			// Parse {N} / {N:spec} placeholders, with {{ and }} as escaped braces
			FString ResultString;
			const TCHAR* FormatData = *FormatStr;
			const int32 FormatLen = FormatStr.Len();
			for (int32 i = 0; i < FormatLen; )
			{
				const TCHAR Char = FormatData[i];
				if (Char == TEXT('{'))
				{
					if (i + 1 < FormatLen && FormatData[i + 1] == TEXT('{'))
					{
						ResultString.AppendChar(TEXT('{'));
						i += 2;
					}
					else
					{
						// Find matching closing brace
						int32 CloseIdx = i + 1;
						while (CloseIdx < FormatLen && FormatData[CloseIdx] != TEXT('}')) CloseIdx++;
						if (CloseIdx < FormatLen)
						{
							// Extract numeric index, ignore optional format spec after ':'
							FString IndexStr;
							for (int32 j = i + 1; j < CloseIdx && FormatData[j] != TEXT(':'); j++)
							{
								IndexStr.AppendChar(FormatData[j]);
							}
							const int32 ArgN = FCString::Atoi(*IndexStr);
							if (ArgN >= 0 && ArgN < Args.Num())
							{
								ResultString.Append(Args[ArgN]);
							}
							i = CloseIdx + 1;
						}
						else
						{
							// Malformed placeholder, keep as-is
							ResultString.AppendChar(Char);
							i++;
						}
					}
				}
				else if (Char == TEXT('}') && i + 1 < FormatLen && FormatData[i + 1] == TEXT('}'))
				{
					ResultString.AppendChar(TEXT('}'));
					i += 2;
				}
				else
				{
					ResultString.AppendChar(Char);
					i++;
				}
			}

			Result = MakeShared<FFormulaValue>(ResultString);
			return true;
		},
		TArray<const FProperty*>(),
		DeclaringType,
		/*bUseClassDefaultObject*/ true);
}

FFormulaFunction FDotNetStringType::GetJoinFunction(UField* DeclaringType)
{
	return FFormulaFunction([](
		const TSharedRef<FFormulaValue>&,
		const FFormulaInvokeArguments& Arguments,
		const UField*,
		const TArray<UField*>*, TSharedPtr<FFormulaValue>& Result)
		{
			// Arg "Separator" at index 0
			const auto SepArg = Arguments.FindArgument(TEXT("Separator"), 0);
			if (!SepArg)
			{
				return false;
			}
			const auto SepValue = SepArg->UpdatedValue.IsValid() ? SepArg->UpdatedValue : SepArg->Value;
			FString Separator;
			SepValue->WriteText(Separator);

			FString ResultString;
			bool bFirst = true;

			// Probe "Values" at index 1 — if it is an array, use the array variant
			if (const auto ValuesArg = Arguments.FindArgument(TEXT("Values"), 1))
			{
				const auto ValuesValue = ValuesArg->UpdatedValue.IsValid()
					? ValuesArg->UpdatedValue
					: ValuesArg->Value;
				if (const FArrayProperty* ArrayProp = CastField<FArrayProperty>(ValuesValue->GetType()))
				{
					void* ArrayAddress = nullptr;
					if (!ValuesValue->TryGetContainerAddress(ArrayAddress) || !ArrayAddress)
					{
						return false;
					}
					FScriptArrayHelper ArrayHelper(ArrayProp, ArrayAddress);
					for (int32 i = 0; i < ArrayHelper.Num(); i++)
					{
						if (!bFirst) ResultString.Append(Separator);
						const FFormulaValue ElemValue(ArrayProp->Inner, ArrayHelper.GetElementPtr(i));
						ElemValue.WriteText(ResultString);
						bFirst = false;
					}
					Result = MakeShared<FFormulaValue>(ResultString);
					return true;
				}
			}

			// Variadic variant: "Arg0", "Arg1", ... at indices 1, 2, ...
			int32 ArgIdx = 0;
			FString ArgName;
			ArgName.Append(TEXT("Arg"));
			ArgName.AppendInt(ArgIdx);
			while (const auto Arg = Arguments.FindArgument(ArgName, ArgIdx + 1))
			{
				if (!bFirst) ResultString.Append(Separator);
				const auto ArgValue = Arg->UpdatedValue.IsValid() ? Arg->UpdatedValue : Arg->Value;
				ArgValue->WriteText(ResultString);
				bFirst = false;
				ArgIdx++;
				ArgName.Reset();
				ArgName.Append(TEXT("Arg"));
				ArgName.AppendInt(ArgIdx);
			}
			Result = MakeShared<FFormulaValue>(ResultString);
			return true;
		},
		TArray<const FProperty*>(),
		DeclaringType,
		/*bUseClassDefaultObject*/ true);
}
