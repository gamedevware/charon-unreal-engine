#include "GameData/Formulas/Expressions/FMemberExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/FFormulaTypeReference.h"
#include "GameData/Formulas/IFormulaTypeDescription.h"

FString GetExpressionMemberName(const TSharedRef<FJsonObject>& ExpressionObj, const bool bRaw)
{
	// Equivalent to ExpressionBuildHelper.GetString logic
	FString MemberName;
	if (!ExpressionObj->TryGetStringField(FFormulaNotation::PROPERTY_OR_FIELD_NAME_ATTRIBUTE, MemberName))
	{
		ExpressionObj->TryGetStringField(FFormulaNotation::NAME_ATTRIBUTE, MemberName);
	}
	if (!bRaw)
	{
		return MemberName.Len() > 0 && MemberName[0] == '@' ? MemberName.RightChop(1) : MemberName;
	}
	return MemberName;
}

FMemberExpression::FMemberExpression(const TSharedRef<FJsonObject>& ExpressionObj) :
	bUseNullPropagation(FExpressionBuildHelper::GetUseNullPropagation(ExpressionObj)),
	Expression(FExpressionBuildHelper::GetExpression(ExpressionObj, FFormulaNotation::EXPRESSION_ATTRIBUTE, true)),
	RawMemberName(GetExpressionMemberName(ExpressionObj, true)),
	MemberName(GetExpressionMemberName(ExpressionObj, false)),
	TypeArguments(FExpressionBuildHelper::GetTypeRefArguments(ExpressionObj, FFormulaNotation::ARGUMENTS_ATTRIBUTE))
{
}

bool FMemberExpression::IsNullPropagationEnabled() const
{
	if (bUseNullPropagation)
	{
		return true;
	}

	if (Expression.IsValid())
	{
		return Expression->IsNullPropagationEnabled();
	}
	return false;
}

FFormulaInvokeResult FMemberExpression::Execute(const FFormulaExecutionContext& Context) const
{
	if (MemberName.IsEmpty())
	{
		return FFormulaInvokeError::ExpressionIsInvalid();
	}

	TSharedPtr<FFormulaTypeReference> StaticTypeReference = nullptr;
	TSharedPtr<IFormulaTypeDescription> StaticTypeDescription = nullptr;
	const FFormulaProperty* FormulaProperty = nullptr;
	TSharedPtr<FFormulaValue> MemberValue;
	if (TryGetTypeReference(StaticTypeReference, /*bSkipSelf*/ true) &&
		(StaticTypeDescription = Context.TypeResolver->GetTypeDescription(StaticTypeReference)).IsValid() &&
		StaticTypeDescription->TryGetProperty(this->MemberName, /* bStatic */ true, FormulaProperty))
	{
		check(FormulaProperty);
		const bool bStaticGetPropertySuccess = FormulaProperty->TryGetValue(FFormulaValue::Null(), MemberValue);
		check(bStaticGetPropertySuccess);

		return MemberValue;
	}
	else if (this->Expression.IsValid())
	{
		const auto Result = this->Expression->Execute(Context);
		if (Result.HasError())
		{
			return Result; // propagate error
		}

		const auto Target = Result.GetValue();
		if (Target->IsNull())
		{
			if (IsNullPropagationEnabled() || Context.AutoNullPropagation)
			{
				return FFormulaValue::Null();
			}
			else
			{
				return FFormulaInvokeError::NullReference();
			}
		}

		const auto TargetTypeDescription = Context.TypeResolver->GetTypeDescription(Target->GetType());
		if (TargetTypeDescription->TryGetProperty(this->MemberName, /* bStatic */ false, FormulaProperty))
		{
			check(FormulaProperty);
			const bool bGetPropertySuccess = FormulaProperty->TryGetValue(Target, MemberValue);
			check(bGetPropertySuccess);

			return MemberValue;
		}

		FString AllMemberNames =  FString::Join(TargetTypeDescription->GetPropertyNames(/* bStatic */ false),TEXT(", "));
		return FFormulaInvokeError::CantFindMember(TargetTypeDescription->GetCPPType(), this->MemberName, AllMemberNames);
	}
	else if (this->RawMemberName == FFormulaNotation::NOTATION_TRUE_STRING)
	{
		return MakeShared<FFormulaValue>(true);
	}
	else if (this->RawMemberName == FFormulaNotation::NOTATION_FALSE_STRING)
	{
		return MakeShared<FFormulaValue>(false);
	}
	else if (this->RawMemberName == FFormulaNotation::NOTATION_NULL_STRING)
	{
		return FFormulaValue::Null();
	}
	else if (const TSharedRef<FFormulaValue>* ArgumentValue = Context.Arguments.Find(this->MemberName))
	{
		return *ArgumentValue;
	}
	else if (const TSharedRef<FFormulaValue>* RawArgumentValue = Context.Arguments.Find(this->RawMemberName))
	{
		return *RawArgumentValue;
	}
	else if (TryGetGlobalMemberValue(MemberValue, Context))
	{
		return MemberValue;
	}
	else if (StaticTypeDescription.IsValid())
	{
		const FString AllStaticMemberNames = FString::Join(StaticTypeDescription->GetPropertyNames(/* bStatic */ true), TEXT(", "));
		return FFormulaInvokeError::CantFindStaticMember(StaticTypeDescription->GetCPPType(), this->MemberName, AllStaticMemberNames);
	}
	else
	{
		TSet<FString> AllGlobalMemberNames;
		GetGlobalMemberNames(AllGlobalMemberNames, Context);
		return FFormulaInvokeError::UnableToResolveGlobalName(MemberName, FString::Join(AllGlobalMemberNames, TEXT(", ")));
	}
}

bool FMemberExpression::TryGetTypeReferenceAndMemberName(TSharedPtr<FFormulaTypeReference>& OutTypeReference,
                                                         FString& OutMemberName) const
{
	OutTypeReference = nullptr;
	OutMemberName.Empty();

	if (TryGetTypeReference(OutTypeReference, /*bSkipSelf*/ true))
	{
		OutMemberName.Append(this->MemberName);
		return true;
	}

	return false;
}

bool FMemberExpression::TryGetTypeReference(TSharedPtr<FFormulaTypeReference>& OutTypeReference,
                                            const bool bSkipSelf) const
{
	OutTypeReference = nullptr;

	if (IsNullPropagationEnabled())
	{
		return false; // type reference can't be .? null propagation
	}

	if (bSkipSelf && this->TypeReferenceSkippingSelf.IsValid())
	{
		OutTypeReference = this->TypeReferenceSkippingSelf;
		return true;
	}
	else if (this->TypeReference.IsValid())
	{
		OutTypeReference = this->TypeReference;
		return true;
	}

	const FMemberExpression* MemberExpression = CastExpression<FMemberExpression>(this->Expression.Get());
	if (!MemberExpression)
	{
		return false; // base expression is not member expression
	}

	if (MemberExpression->TryGetTypeReference(OutTypeReference, /*bSkipSelf*/ false))
	{
		return false; // base expression is not a type reference
	}

	if (bSkipSelf)
	{
		this->TypeReferenceSkippingSelf = OutTypeReference;
	}
	else
	{
		this->TypeReference = OutTypeReference = MakeShared<FFormulaTypeReference>(this->MemberName);
	}

	return !!OutTypeReference;
}

bool FMemberExpression::TryGetGlobalMemberValue(TSharedPtr<FFormulaValue>& MemberValue,
                                                const FFormulaExecutionContext& Context) const
{
	if (Context.Global->IsNull())
	{
		return false;
	}

	const FFormulaProperty* FormulaProperty = nullptr;
	const auto GlobalType = Context.TypeResolver->GetTypeDescription(Context.Global->GetType());
	if (GlobalType->TryGetProperty(this->MemberName, /* bStatic */ false, FormulaProperty))
	{
		check(FormulaProperty);
		const bool bGetPropertySuccess = FormulaProperty->TryGetValue(Context.Global, MemberValue);
		check(bGetPropertySuccess);

		return true;
	}

	return false;
}

void FMemberExpression::GetGlobalMemberNames(TSet<FString>& MemberNames, const FFormulaExecutionContext& Context)
{
	if (!Context.Global->IsNull())
	{
		const auto GlobalType = Context.TypeResolver->GetTypeDescription(Context.Global->GetType());
		MemberNames.Append(GlobalType->GetPropertyNames(/*bStatic*/ false));
	}
	for (auto ArgumentPair : Context.Arguments)
	{
		MemberNames.Add(ArgumentPair.Key);
	}
}
