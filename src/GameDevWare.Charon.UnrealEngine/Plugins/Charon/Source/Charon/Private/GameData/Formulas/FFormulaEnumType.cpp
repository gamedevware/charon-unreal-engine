// Copyright GameDevWare, Denis Zykov 2025

#include "FFormulaEnumType.h"

#include "FDotNetSurrogateType.h"

FFormulaEnumType::FFormulaEnumType(UEnum* Enum, const TSharedRef<IFormulaType>& UnderlyingType) :
	EnumPtr(Enum),
	UnderlyingType(UnderlyingType)
{
	check(Enum);

	for (int32 i = 0; i < Enum->NumEnums(); ++i)
	{
		TSharedRef<FFormulaValue> LiteralValue = MakeShared<FFormulaValue>(Enum->GetValueByIndex(i));
		FString LiteralName = Enum->GetNameByIndex(i).ToString();

		this->EnumLiteralNames.Add(LiteralName);
		this->EnumLiterals.Add(LiteralName, FFormulaProperty(
			UDotNetInt64::GetLiteralProperty(),
			[LiteralValue](const TSharedRef<FFormulaValue>&,
						  TSharedPtr<FFormulaValue>& Result) -> bool
			{
				Result = LiteralValue;
				return true;
			},
			nullptr,
			Enum,
			/*bUseClassDefaultObject*/ false));
	}
}

bool FFormulaEnumType::IsAssignableFrom(UField* Type) const
{
	const UEnum* OtherEnum = Cast<UEnum>(Type);
	return OtherEnum && OtherEnum == EnumPtr.Get();
}

FString FFormulaEnumType::GetCPPType() const
{
	return TEXT("E") + EnumPtr->GetName() + TEXT(": ") + UnderlyingType->GetCPPType();
}

TSharedPtr<IFormulaType> FFormulaEnumType::GetUnderlyingType() const
{
	return this->UnderlyingType;
}

const TArray<FString>& FFormulaEnumType::GetPropertyNames(bool bStatic)
{
	if (this->EnumLiteralNames.Num() == 0)
	{
		const UEnum* Enum = this->EnumPtr.Get();
		for (int32 i = 0; i < Enum->NumEnums(); ++i)
		{
			FString LiteralName = Enum->GetNameByIndex(i).ToString();
			this->EnumLiteralNames.Add(LiteralName);
		}
	}
	return this->EnumLiteralNames;
}

const TArray<FString>& FFormulaEnumType::GetFunctionNames(bool bStatic)
{
	if (this->FunctionNames.Num() == 0)
	{
		this->FunctionNames.Add(TEXT("ToString"));
		this->FunctionNames.Add(TEXT("GetType"));
		this->FunctionNames.Add(TEXT("Equals"));
	}
	return this->FunctionNames;
}

FFormulaFunction FFormulaEnumType::GetToStringFunction() const
{
	const UEnum* Enum = EnumPtr.Get();
	return FFormulaFunction([&Enum](const TSharedRef<FFormulaValue>& Target,
		FFormulaInvokeArguments&, const UField*, const TArray<UField*>*,
		TSharedPtr<FFormulaValue>& Result)
		{
			int64 EnumValue = 0;
			if (!Target->TryGetInt64(EnumValue))
			{
				return false;
			}
			if (!Enum)
			{
				return false;
			}
			const FName LabelName = Enum->GetNameByValue(EnumValue);
			if (LabelName == NAME_None)
			{
				Result = MakeShared<FFormulaValue>(FString::Printf(TEXT("%lld"), EnumValue));
				return true;
			}
			else
			{
				Result = MakeShared<FFormulaValue>(LabelName.ToString());
				return true;
			}
		},
		TArray<const FProperty*>(),
		EnumPtr.Get(),
		/*bUseClassDefaultObject*/ false);
}

bool FFormulaEnumType::TryGetFunction(
	const FString& MemberName,
	bool bStatic,
	const FFormulaFunction*& FoundFunction)
{
	if (bStatic)
	{
		FoundFunction = nullptr;
		return false;
	}

	if (this->Functions.Num() == 0)
	{
		this->Functions.Add(TEXT("ToString"), GetToStringFunction());
		this->Functions.Add(TEXT("GetType"), FDotNetSurrogateType::GetGetTypeFunction(EnumPtr.Get()));
		this->Functions.Add(TEXT("Equals"), FDotNetSurrogateType::GetEqualsFunction(EnumPtr.Get()));
	}

	FoundFunction = this->Functions.Find(MemberName);
	return FoundFunction != nullptr;
}

bool FFormulaEnumType::TryGetProperty(
	const FString& MemberName,
	bool bStatic, const
	FFormulaProperty*& FoundMember)
{
	if (!bStatic)
	{
		FoundMember = nullptr;
		return false;
	}

	if (this->EnumLiterals.Num() == 0)
	{
		UEnum* Enum = this->EnumPtr.Get();
		for (int32 i = 0; i < Enum->NumEnums(); ++i)
		{
			TSharedRef<FFormulaValue> LiteralValue = MakeShared<FFormulaValue>(Enum->GetValueByIndex(i));
			FString LiteralName = Enum->GetNameByIndex(i).ToString();

			this->EnumLiterals.Add(LiteralName, FFormulaProperty(
				UDotNetInt64::GetLiteralProperty(),
				[LiteralValue](const TSharedRef<FFormulaValue>&,
							  TSharedPtr<FFormulaValue>& Result) -> bool
				{
					Result = LiteralValue;
					return true;
				},
				nullptr,
				Enum,
				/*bUseClassDefaultObject*/ false));
		}
	}

	FoundMember = this->EnumLiterals.Find(MemberName);
	return FoundMember != nullptr;
}
