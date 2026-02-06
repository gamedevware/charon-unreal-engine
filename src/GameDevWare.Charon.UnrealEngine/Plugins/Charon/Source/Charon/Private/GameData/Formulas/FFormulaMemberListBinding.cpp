#include "GameData/Formulas/Expressions/FFormulaMemberListBinding.h"
#include "GameData/Formulas/Expressions/FFormulaElementInitBinding.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/FFormulaProperty.h"

FFormulaMemberListBinding::FFormulaMemberListBinding(const TSharedRef<FJsonObject>& ExpressionObj) :
	FFormulaMemberBinding(GetExpressionRawMemberName(ExpressionObj)),
	Initializers(FExpressionBuildHelper::GetElementInitBindings(ExpressionObj, FFormulaNotation::INITIALIZERS_ATTRIBUTE))
{
}

FFormulaMemberListBinding::FFormulaMemberListBinding(const FString& RawMemberName,
                                                     const TArray<TSharedPtr<FFormulaElementInitBinding>>& Initializers) :
	FFormulaMemberBinding(RawMemberName),
	Initializers(Initializers)
{
}

bool FFormulaMemberListBinding::IsValid() const
{
	if (this->RawMemberName.IsEmpty() || this->MemberName.IsEmpty())
	{
		return false;
	}
	for (const auto& Initializer : Initializers)
	{
		if (!Initializer.IsValid())
		{
			return false;
		}
	}
	return true;
}

FFormulaExecutionResult FFormulaMemberListBinding::ApplyToMember(const TSharedRef<FFormulaValue>& Target,
                                                                      const FFormulaProperty* Member,
                                                                      const FFormulaExecutionContext& Context) const
{
	check(Member);
	
	TSharedPtr<FFormulaValue> MemberValue;
	if (!Member->TryGetValue(Target, MemberValue) || !MemberValue.IsValid())
	{
		return FFormulaExecutionError::MemberAccessFailed(Target->GetCPPType(), this->MemberName);
	}

	for (const auto& Initializer : Initializers)
	{
		FFormulaExecutionResult InitResult = Initializer->Apply(MemberValue.ToSharedRef(), Context);
		if (InitResult.HasError())
		{
			return InitResult;
		}
	}
	
	// This will cause the Map/Set to be re-hashed after some element is added.
	// This isn't optimal, but trying to optimize this case would take
	// more time than I'd like to devote to it.
	FFormulaElementInitBinding::CompleteCollectionInitialization(MemberValue.ToSharedRef());
	
	// Since we copied collection into MemberValue, now we need to save updated value back 
	if (!Member->TrySetValue(Target, MemberValue))
	{
		return FFormulaExecutionError::MemberAccessFailed(Target->GetCPPType(), this->MemberName);
	}
	
	return Target;
}

void FFormulaMemberListBinding::DebugPrintTo(FString& OutValue) const
{
	OutValue.Append(this->RawMemberName);
	OutValue.Append(TEXT(": { "));
	bool bFirstItem = true;
	for (const auto& Initializer : Initializers)
	{
		if (!bFirstItem)
		{
			OutValue.Append(TEXT(", "));
		}
		bFirstItem = false;
		
		Initializer->DebugPrintTo(OutValue);
	}
	OutValue.Append(TEXT(" }"));
}
