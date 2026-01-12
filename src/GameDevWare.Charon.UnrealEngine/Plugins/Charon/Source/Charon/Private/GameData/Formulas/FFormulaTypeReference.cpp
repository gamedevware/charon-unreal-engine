#include "GameData/Formulas/FFormulaTypeReference.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaConstants.h"

DEFINE_LOG_CATEGORY(LogFormulaTypeReference);

FFormulaTypeReference::FFormulaTypeReference(const TSharedRef<FJsonObject>& ExpressionObj)
{
	TypeArguments = FExpressionBuildHelper::GetTypeRefArguments(ExpressionObj, FormulaConstants::ARGUMENTS_ATTRIBUTE);
	Expression = FExpressionBuildHelper::GetTypeRef(ExpressionObj, FormulaConstants::EXPRESSION_ATTRIBUTE, true);
	Name = FExpressionBuildHelper::GetString(ExpressionObj, FormulaConstants::NAME_ATTRIBUTE);
		
	if (TypeArguments.Num() > 0)
	{
		UE_LOG(LogFormulaTypeReference, Warning, TEXT("TypeArguments detected for %s. Unreal Engine C++ backend doesn't supports generics."), *Name);
	}
}

FFormulaTypeReference::FFormulaTypeReference(FString Name)
{
	this->Name = Name;
}

bool FFormulaTypeReference::IsEmpty() const
{
	return Name.IsEmpty() && !Expression.IsValid() && TypeArguments.Num() == 0;
}

FString FFormulaTypeReference::GetFullName(bool bWriteGenerics) const
{
	if (FullName.IsEmpty())
	{
		TStringBuilder<256> Builder;
		BuildFullNameInternal(Builder, bWriteGenerics);
		FullName = Builder.ToString();
	}
	return FullName;
}

void FFormulaTypeReference::BuildFullNameInternal(FStringBuilderBase& Builder, bool bWriteGenerics) const
{
	if (Expression.IsValid())
	{
		Expression->BuildFullNameInternal(Builder, bWriteGenerics);
		Builder.Append(TEXT("."));
	}

	Builder.Append(Name);

	if (bWriteGenerics && TypeArguments.Num() > 0)
	{
		Builder.Append(TEXT("<"));
		for (int32 i = 0; i < TypeArguments.Num(); ++i)
		{
			TypeArguments[i]->BuildFullNameInternal(Builder, bWriteGenerics);
			if (i < TypeArguments.Num() - 1)
			{
				Builder.Append(TEXT(", "));
			}
		}
		Builder.Append(TEXT(">"));
	}
}
