#include "GameData/Formulas/Expressions/FFormulaMemberMemberBinding.h"

#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/FFormulaProperty.h"
#include "GameData/Formulas/IFormulaType.h"

FFormulaMemberMemberBinding::FFormulaMemberMemberBinding(const TSharedRef<FJsonObject>& ExpressionObj) :
	FFormulaMemberBinding(RawMemberName),
	Bindings(FExpressionBuildHelper::GetBindings(ExpressionObj, FFormulaNotation::BINDINGS_ATTRIBUTE))
{
}

FFormulaMemberMemberBinding::FFormulaMemberMemberBinding(const FString& RawMemberName,
                                                         const TArray<TSharedPtr<FFormulaMemberBinding>>& Bindings) :
	FFormulaMemberBinding(RawMemberName),
	Bindings(Bindings)
{
}

bool FFormulaMemberMemberBinding::IsValid() const
{
	if (this->RawMemberName.IsEmpty() || this->MemberName.IsEmpty())
	{
		return false;
	}
	for (auto Binding : this->Bindings)
	{
		if (!Binding.IsValid())
		{
			return false;
		}
	}
	return true;
}

FFormulaExecutionResult FFormulaMemberMemberBinding::ApplyMemberChanges(const TSharedRef<FFormulaValue>& Target,
                                                                        const FFormulaProperty* Member,
                                                                        const FFormulaExecutionContext& Context) const
{
	check(Member);

	TSharedPtr<FFormulaValue> MemberValue;
	if (!Member->TryGetValue(Target, MemberValue))
	{
		return FFormulaExecutionError::MemberAccessFailed(Target->GetCPPType(), this->MemberName);
	}
	
	for (const auto FormulaMemberBinding : this->Bindings)
	{
		auto ApplyResult = FormulaMemberBinding->Apply(MemberValue.ToSharedRef(), Context);
		if (ApplyResult.HasError())
		{
			return ApplyResult;
		}
	}
	return Target;
}

void FFormulaMemberMemberBinding::DebugPrintTo(FString& OutValue) const
{
	OutValue.Append(this->RawMemberName);
	OutValue.Append(TEXT(": { "));
	bool bFirstBindings = true;
	for (auto Binding : this->Bindings)
	{
		if (!bFirstBindings)
		{
			OutValue.Append(TEXT(", "));
		}
		bFirstBindings = false;
		
		if (Binding.IsValid())
		{
			Binding->DebugPrintTo(OutValue);
		}
		else
		{
			OutValue.Append(TEXT("#INVALID#"));
		}
	}
	OutValue.Append(TEXT("} "));
}
