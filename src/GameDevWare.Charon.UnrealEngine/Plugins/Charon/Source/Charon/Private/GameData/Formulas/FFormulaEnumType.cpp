// Copyright GameDevWare, Denis Zykov 2025

#include "FFormulaEnumType.h"

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
			[LiteralValue](const TSharedRef<FFormulaValue>&, TSharedPtr<FFormulaValue>& Result) -> bool
			{
				Result = LiteralValue;
				return true;
			},
			nullptr,
			Enum,
		    /*bUseClassDefaultObject*/ false));
	}

	this->FunctionNames.Add(TEXT("ToString"));
	this->Functions.Add(TEXT("ToString"), FFormulaFunction(
		[this](const TSharedRef<FFormulaValue>& Target,
		FFormulaInvokeArguments&, const UField*, const TArray<UField*>*,
		TSharedPtr<FFormulaValue>& Result)
		{
			return this->EnumToString(Target, Result);
		},
		Enum,
		/*bUseClassDefaultObject*/ false));
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
	return this->EnumLiteralNames;
}

const TArray<FString>& FFormulaEnumType::GetFunctionNames(bool bStatic)
{
	return this->FunctionNames;
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

	FoundFunction = this->Functions.Find(MemberName);
	return FoundFunction != nullptr;
}

bool FFormulaEnumType::TryGetProperty(
	const FString& MemberName,
	bool bStatic,const
	FFormulaProperty*& FoundMember)
{
	if (!bStatic)
	{
		FoundMember = nullptr;
		return false;
	}

	FoundMember = this->EnumLiterals.Find(MemberName);
	return FoundMember != nullptr;
}

bool FFormulaEnumType::EnumToString(
	const TSharedRef<FFormulaValue>& Target,
	TSharedPtr<FFormulaValue>& Result) const
{
	int64 EnumValue = 0;
	if (!Target->TryGetInt64(EnumValue))
	{
		return false;
	}
	const UEnum* Enum = this->EnumPtr.Get();
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
}
