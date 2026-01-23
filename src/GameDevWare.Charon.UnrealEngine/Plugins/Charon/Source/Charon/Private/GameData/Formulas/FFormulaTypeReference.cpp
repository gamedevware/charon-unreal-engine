#include "GameData/Formulas/FFormulaTypeReference.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"

DEFINE_LOG_CATEGORY(LogFormulaTypeReference);

FFormulaTypeReference::FFormulaTypeReference(const TSharedRef<FJsonObject>& ExpressionObj):
	Expression(FExpressionBuildHelper::GetTypeRef(ExpressionObj, FFormulaNotation::EXPRESSION_ATTRIBUTE, true)),
	TypeArguments(FExpressionBuildHelper::GetTypeRefArguments(ExpressionObj, FFormulaNotation::ARGUMENTS_ATTRIBUTE)),
	Name(FExpressionBuildHelper::GetString(ExpressionObj, FFormulaNotation::NAME_ATTRIBUTE))
{
	
}

FFormulaTypeReference::FFormulaTypeReference(const FString& Name):
	Expression(nullptr), Name(Name)
{
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

		if (bWriteGenerics)
		{
			FullNameWithGenerics = Builder.ToString();
		}
		else
		{
			FullName = Builder.ToString();
		}
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
