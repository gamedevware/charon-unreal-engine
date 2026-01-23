#include "FFormulaEnumDescription.h"

FFormulaPropertyGetterFunc CreateEnumLabelGetter(TSharedRef<FFormulaValue> LabelValue)
{
	return FFormulaPropertyGetterFunc();
}

FFormulaEnumDescription::FFormulaEnumDescription(UEnum* Enum) :
	EnumPtr(Enum)
{
	check(Enum);

	for (int32 i = 0; i < Enum->NumEnums(); ++i)
	{
		TSharedRef<FFormulaValue> LiteralValue = MakeShared<FFormulaValue>(Enum->GetValueByIndex(i));
		FString LiteralName = Enum->GetNameByIndex(i).ToString();

		this->EnumLiteralNames.Add(LiteralName);
		this->EnumLiterals.Add(LiteralName, FFormulaProperty(
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
		const TMap<FString, TSharedRef<FFormulaValue>>&,
		const IFormulaTypeDescription*,
		const TArray<UField*>&,
		TSharedPtr<FFormulaValue>& Result)
		{
			return this->EnumToString(Target, Result);
		},
		Enum,
		/*bUseClassDefaultObject*/ false));
}

bool FFormulaEnumDescription::IsAssignableFrom(UField* Type) const
{
	const UEnum* OtherEnum = Cast<UEnum>(Type);
	return OtherEnum && OtherEnum == EnumPtr.Get();
}

FString FFormulaEnumDescription::GetCPPType() const
{
	return TEXT("E") + EnumPtr->GetName();
}

const TArray<FString>& FFormulaEnumDescription::GetPropertyNames(bool bStatic)
{
	return this->EnumLiteralNames;
}

const TArray<FString>& FFormulaEnumDescription::GetFunctionNames(bool bStatic)
{
	return this->FunctionNames;
}

bool FFormulaEnumDescription::TryGetFunction(
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

bool FFormulaEnumDescription::TryGetProperty(
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

bool FFormulaEnumDescription::EnumToString(
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
