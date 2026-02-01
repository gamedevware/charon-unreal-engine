#include "GameData/Formulas/Expressions/FFormulaMemberBinding.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/FFormulaProperty.h"
#include "GameData/Formulas/IFormulaType.h"

FFormulaMemberBinding::FFormulaMemberBinding(const FString& RawMemberName):
	RawMemberName(RawMemberName),
	MemberName(GetMemberName(RawMemberName))
{
}

FFormulaExecutionResult FFormulaMemberBinding::Apply(const TSharedRef<FFormulaValue>& Target,
                                                     const FFormulaExecutionContext& Context) const
{
	if (!this->IsValid())
	{
		return FFormulaExecutionError::ExpressionIsInvalid();
	}
	
	const FFormulaProperty* FormulaProperty = nullptr;
	const auto TargetType = Context.TypeResolver->GetType(Target);
	if (TargetType->TryGetProperty(this->MemberName, /* bStatic */ false, FormulaProperty))
	{
		check(FormulaProperty);
		return this->ApplyToMember(Target, FormulaProperty, Context);
	}

	FString AllMemberNames =  FString::Join(TargetType->GetPropertyNames(/* bStatic */ false),TEXT(", "));
	return FFormulaExecutionError::CantFindMember(TargetType->GetCPPType(), this->MemberName, AllMemberNames);
}

FString FFormulaMemberBinding::GetExpressionRawMemberName(const TSharedRef<FJsonObject>& ExpressionObj)
{
	FString MemberName;
	if (!ExpressionObj->TryGetStringField(FFormulaNotation::PROPERTY_OR_FIELD_NAME_ATTRIBUTE, MemberName))
	{
		ExpressionObj->TryGetStringField(FFormulaNotation::NAME_ATTRIBUTE, MemberName);
	}
	return MemberName;
}

FString FFormulaMemberBinding::GetMemberName(FString RawMemberName)
{
	return RawMemberName.Len() > 0 && RawMemberName[0] == '@' ? RawMemberName.RightChop(1) : RawMemberName;
}
