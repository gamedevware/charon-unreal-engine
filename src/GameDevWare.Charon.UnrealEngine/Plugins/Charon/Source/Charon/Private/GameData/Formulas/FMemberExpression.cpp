// Copyright GameDevWare, Denis Zykov 2025

#include "GameData/Formulas/Expressions/FMemberExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/FFormulaTypeReference.h"
#include "GameData/Formulas/IFormulaType.h"



FMemberExpression::FMemberExpression(const TSharedRef<FJsonObject>& ExpressionObj) :
	bUseNullPropagation(FExpressionBuildHelper::GetUseNullPropagation(ExpressionObj)),
	Expression(FExpressionBuildHelper::GetExpression(ExpressionObj, FFormulaNotation::EXPRESSION_ATTRIBUTE, true)),
	RawMemberName(GetExpressionRawMemberName(ExpressionObj)),
	MemberName(GetMemberName(GetExpressionRawMemberName(ExpressionObj))),
	TypeArguments(FExpressionBuildHelper::GetTypeRefArguments(ExpressionObj, FFormulaNotation::ARGUMENTS_ATTRIBUTE))
{
}

FMemberExpression::FMemberExpression(const TSharedPtr<FFormulaExpression>& Expression, const FString& RawMemberName,
	const TArray<TSharedPtr<FFormulaTypeReference>>& TypeArguments, const bool bUseNullPropagation) :
	bUseNullPropagation(bUseNullPropagation), Expression(Expression),
	RawMemberName(RawMemberName), MemberName(GetMemberName(RawMemberName)),
	TypeArguments(TypeArguments)
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

FFormulaExecutionResult FMemberExpression::Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const
{
	if (!this->IsValid())
	{
		return FFormulaExecutionError::ExpressionIsInvalid();
	}

	TSharedPtr<FFormulaTypeReference> StaticTypeReference = nullptr;
	TSharedPtr<IFormulaType> StaticTypeDescription = nullptr;
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
		const auto Result = this->Expression->Execute(Context, nullptr);
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
				return FFormulaExecutionError::NullReference();
			}
		}

		const auto TargetType = Context.TypeResolver->GetTypeDescription(Target->GetType());
		if (TargetType->TryGetProperty(this->MemberName, /* bStatic */ false, FormulaProperty))
		{
			check(FormulaProperty);
			const bool bGetPropertySuccess = FormulaProperty->TryGetValue(Target, MemberValue);
			if (!bGetPropertySuccess)
			{
				return FFormulaExecutionError::MemberAccessFailed(TargetType->GetCPPType(), this->MemberName);
			}
			return MemberValue;
		}

		FString AllMemberNames =  FString::Join(TargetType->GetPropertyNames(/* bStatic */ false),TEXT(", "));
		return FFormulaExecutionError::CantFindMember(TargetType->GetCPPType(), this->MemberName, AllMemberNames);
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
		return FFormulaExecutionError::CantFindStaticMember(StaticTypeDescription->GetCPPType(), this->MemberName, AllStaticMemberNames);
	}
	else
	{
		TSet<FString> AllGlobalMemberNames;
		GetGlobalMemberNames(AllGlobalMemberNames, Context);
		return FFormulaExecutionError::UnableToResolveGlobalName(MemberName, FString::Join(AllGlobalMemberNames, TEXT(", ")));
	}
}

bool FMemberExpression::TryGetTypeReferenceAndMemberName(TSharedPtr<FFormulaTypeReference>& OutTypeReference,
                                                         FString& OutMemberName) const
{
	OutTypeReference = nullptr;
	OutMemberName.Reset();

	if (TryGetTypeReference(OutTypeReference, /*bSkipSelf*/ true))
	{
		OutMemberName.Append(this->MemberName);
		return true;
	}

	return false;
}

bool FMemberExpression::IsValid() const
{
	if (this->MemberName.IsEmpty())
	{
		return false;
	}
	for (auto ArgumentPair : this->TypeArguments)
	{
		if (!ArgumentPair.IsValid())
		{
			return false;
		}
	}
	return true;
}

void FMemberExpression::DebugPrintTo(FString& OutValue) const
{
	if (this->Expression.IsValid())
	{
		this->Expression->DebugPrintTo(OutValue);
		if (this->bUseNullPropagation)
		{
			OutValue.Append("?");
		}
		OutValue.Append(".");
	}
	OutValue.Append(this->MemberName);
	
	if (this->TypeArguments.Num() > 0)
	{
		OutValue.Append("<");
		bool bFirstArgument = true;;
		for (auto Argument : this->TypeArguments)
		{
			if (!bFirstArgument)
			{
				OutValue.Append(", ");
			}
			bFirstArgument = false;

			if (Argument.IsValid())
			{
				OutValue.Append(Argument->GetFullName(true));
			}
			else
			{
				OutValue.Append(TEXT("#INVALID#"));
			}
		}
		OutValue.Append(">");
	}
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

	const FMemberExpression* BaseMemberExpression = CastExpression<FMemberExpression>(this->Expression.Get());
	if (bSkipSelf)
	{
		if (!BaseMemberExpression || !BaseMemberExpression->TryGetTypeReference(OutTypeReference, /*bSkipSelf*/ false))
		{
			return false; // base expression is not a type reference
		}
		
		this->TypeReferenceSkippingSelf = OutTypeReference;
	}
	else
	{
		if (BaseMemberExpression && BaseMemberExpression->TryGetTypeReference(OutTypeReference, /*bSkipSelf*/ false))
		{
			// like System.Array
			this->TypeReference = OutTypeReference = MakeShared<FFormulaTypeReference>(this->MemberName, OutTypeReference, this->TypeArguments);
		}
		else if (!BaseMemberExpression)
		{
			// like Array
			this->TypeReference = OutTypeReference = MakeShared<FFormulaTypeReference>(this->MemberName, this->TypeArguments);
		}
		else
		{
			return false; // base expression is not a type reference
		}
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
