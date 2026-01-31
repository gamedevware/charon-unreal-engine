#include "GameData/Formulas/Expressions/FFormulaMemberListBinding.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/FFormulaProperty.h"
#include "GameData/Formulas/Expressions/FFormulaExpression.h"

FFormulaMemberListBinding::FFormulaMemberListBinding(const TSharedRef<FJsonObject>& ExpressionObj) :
	FFormulaMemberBinding(RawMemberName),
	ElementInit(FFormulaElementInitBinding(FExpressionBuildHelper::GetArgumentsList(ExpressionObj, FFormulaNotation::INITIALIZERS_ATTRIBUTE)))
{
}

FFormulaMemberListBinding::FFormulaMemberListBinding(const FString& RawMemberName,
                                                     const TArray<TSharedPtr<FFormulaExpression>>& Initializers) :
	FFormulaMemberBinding(RawMemberName),
	ElementInit(FFormulaElementInitBinding(Initializers))
{
}

bool FFormulaMemberListBinding::IsValid() const
{
	if (this->RawMemberName.IsEmpty() || this->MemberName.IsEmpty())
	{
		return false;
	}
	return this->ElementInit.IsValid();;
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
	
	FFormulaExecutionResult InitResult = this->ElementInit.Apply(MemberValue.ToSharedRef(), Context);
	if (InitResult.HasError())
	{
		return InitResult;
	}
	
	// This will cause the Map/Set to be re-hashed after each element is added.
	// This isn't optimal, but trying to optimize this case would take
	// more time than I'd like to devote to it.
	FFormulaElementInitBinding::CompleteCollectionInitialization(MemberValue.ToSharedRef());
	
	// Since we copied collection into MemberValue, now we need to save updated value back 
	if (!Member->TrySetValue(Target, MemberValue))
	{
		return FFormulaExecutionError::MemberAccessFailed(Target->GetCPPType(), this->MemberName);
	}
	
	return InitResult;
}

void FFormulaMemberListBinding::DebugPrintTo(FString& OutValue) const
{
	OutValue.Append(this->RawMemberName);
	OutValue.Append(TEXT(": { "));
	this->ElementInit.DebugPrintTo(OutValue);
	OutValue.Append(TEXT("} "));
}
