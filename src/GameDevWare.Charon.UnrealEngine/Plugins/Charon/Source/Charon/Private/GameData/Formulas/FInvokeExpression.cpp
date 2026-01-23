#include "GameData/Formulas/Expressions/FInvokeExpression.h"
#include "GameData/Formulas/Expressions/FFormulaExpression.h"
#include "GameData/Formulas/Expressions/FMemberExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaFunction.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/IFormulaTypeDescription.h"
#include "GameData/Formulas/FFormulaTypeReference.h"

FInvokeExpression::FInvokeExpression(const TSharedRef<FJsonObject>& ExpressionObj):
	Arguments(FExpressionBuildHelper::GetArguments(ExpressionObj, FFormulaNotation::ARGUMENTS_ATTRIBUTE)),
	Expression(FExpressionBuildHelper::GetExpression(ExpressionObj, FFormulaNotation::EXPRESSION_ATTRIBUTE))
{
}

bool FInvokeExpression::IsNullPropagationEnabled() const
{
	if (Expression.IsValid())
	{
		return Expression->IsNullPropagationEnabled();
	}
	return false;
}

FFormulaInvokeResult FInvokeExpression::Execute(const FFormulaExecutionContext& Context) const
{
	TArray<UField*> TypeArguments;
	FString MemberName;
	TSharedPtr<FFormulaTypeReference> StaticTypeReference = nullptr;
	TSharedPtr<IFormulaTypeDescription> StaticTypeDescription = nullptr;
	const FFormulaFunction* FormulaFunction = nullptr;
	const FMemberExpression* MemberExpression = CastExpression<FMemberExpression>(this->Expression.Get());

	// resolve type arguments
	for (auto TypeArgumentReference : MemberExpression->TypeArguments)
	{
		auto TypeArgument = Context.TypeResolver->GetTypeDescription(TypeArgumentReference);
		if (!TypeArgument->GetTypeClassOrStruct())
		{
			return FFormulaInvokeError::UnableToResolveType(TypeArgumentReference->GetFullName(/* include generics */ true));
		}
			
		TypeArguments.Add(TypeArgument->GetTypeClassOrStruct());
	}
	//
	
	if (this->Expression.IsValid() && !MemberExpression)
	{
		// ex. (a + b)(param1, param2)
		const auto Result = this->ExecuteExpression(this->Expression.ToSharedRef(), Context);
		if (Result.HasError())
		{
			return Result; // propagate error
		}
		MemberName.Empty();
		MemberName.Append(TEXT("Invoke"));
		
		return FindAndInvokeFunction(Result.GetValue(), MemberName, TypeArguments, Context);
	}
	else if (MemberExpression && MemberExpression->TryGetTypeReferenceAndMemberName(StaticTypeReference, MemberName) &&
		(StaticTypeDescription = Context.TypeResolver->GetTypeDescription(StaticTypeReference)).IsValid() &&
		StaticTypeDescription->TryGetFunction(MemberName, /* bStatic */ true, FormulaFunction))
	{
		// ex. System.Math.Add()
		return InvokeFunction(FFormulaValue::Null(), FormulaFunction, MemberName, TypeArguments, Context);
	}
	else if (MemberExpression && MemberExpression->Expression.IsValid())
	{
		// ex. value.Add(1, 2)
		auto Result = this->ExecuteExpression(MemberExpression->Expression.ToSharedRef(), Context);
		if (Result.HasError())
		{
			return Result; // propagate error
		}
		return FindAndInvokeFunction(Result.GetValue(), MemberExpression->MemberName, TypeArguments, Context);
	}
	else if (MemberExpression && !Context.Global->IsNull())
	{
		// ex. hello()
		return FindAndInvokeFunction(Context.Global, MemberExpression->MemberName, TypeArguments, Context);
	}
	else if (StaticTypeDescription.IsValid())
	{
		const FString AllStaticMemberNames = FString::Join(StaticTypeDescription->GetFunctionNames(/* bStatic */ true), TEXT(", "));
		return FFormulaInvokeError::CantFindStaticMember(StaticTypeDescription->GetCPPType(), MemberName, AllStaticMemberNames);
	}
	else
	{
		const TSet<FString> AllGlobalFunctionNames;
		GetGlobalFunctionNames(AllGlobalFunctionNames, Context);
		return FFormulaInvokeError::UnableToResolveGlobalName(MemberName, FString::Join(AllGlobalFunctionNames, TEXT(", ")));
	}
}

FFormulaInvokeResult FInvokeExpression::ExecuteExpression(const TSharedRef<FFormulaExpression>& TargetExpression, const FFormulaExecutionContext& Context) const
{
	const bool IsNullPropagation = this->IsNullPropagationEnabled() || Context.AutoNullPropagation;
	const auto TargetResult = TargetExpression->Execute(Context);
	if (TargetResult.HasError())
	{
		return TargetResult; // propagate error
	}

	const auto Target = TargetResult.GetValue();
	if (Target->IsNull())
	{
		if (IsNullPropagation)
		{
			return FFormulaValue::Null();
		}
		else
		{
			return FFormulaInvokeError::NullReference();
		}
	}
	return TargetResult; 
}

FFormulaInvokeResult FInvokeExpression::FindAndInvokeFunction(const TSharedRef<FFormulaValue>& Target, const FString& FunctionName, const TArray<UField*>& TypeArguments, const FFormulaExecutionContext& Context) const
{
	const FFormulaFunction* FormulaFunction = nullptr;
	const auto TargetTypeDescription = Context.TypeResolver->GetTypeDescription(Target->GetType());
	if (TargetTypeDescription->TryGetFunction(FunctionName, /* bStatic */ false, FormulaFunction))
	{
		check(FormulaFunction);

		return InvokeFunction(Target, FormulaFunction, FunctionName, TypeArguments, Context);
	}

	FString AllFunctionNames =  FString::Join(TargetTypeDescription->GetFunctionNames(/* bStatic */ false),TEXT(", "));
	return FFormulaInvokeError::CantFindMember(TargetTypeDescription->GetCPPType(), FunctionName, AllFunctionNames);
}

FFormulaInvokeResult FInvokeExpression::InvokeFunction(const TSharedRef<FFormulaValue>& Target,	const FFormulaFunction* FormulaFunction, const FString& FunctionName, const TArray<UField*>& TypeArguments, const FFormulaExecutionContext& Context) const
{
	TMap<FString, TSharedRef<FFormulaValue>> CallArguments;
	for (auto ArgumentPair : this->Arguments)
	{
		const auto ArgumentResult = ArgumentPair.Value->Execute(Context);
		if (ArgumentResult.HasError())
		{
			return ArgumentResult;
		}
		const auto ArgumentValue = ArgumentResult.GetValue();
		CallArguments.Add(ArgumentPair.Key, ArgumentValue);
	}
		
	TSharedPtr<FFormulaValue> InvokeValue;
	if (FormulaFunction->TryInvoke(Target, CallArguments, nullptr, TypeArguments, InvokeValue))
	{
		return InvokeValue;
	}

	// binding failed
	TArray<FString> ParameterTypeNames;
	for (auto CallArgumentPair : CallArguments)
	{
		ParameterTypeNames.Add(CallArgumentPair.Value->GetType()->GetCPPType());
	}
	return FFormulaInvokeError::UnableToBindMethodToParameters(Target->GetType()->GetCPPType(), FunctionName, FString::Join(ParameterTypeNames, TEXT(", ")), CallArguments.Num());
}

void FInvokeExpression::GetGlobalFunctionNames(TSet<FString> FunctionNames, const FFormulaExecutionContext& Context)
{
	if (!Context.Global->IsNull())
	{
		const auto GlobalType = Context.TypeResolver->GetTypeDescription(Context.Global->GetType());
		FunctionNames.Append(GlobalType->GetFunctionNames(/*bStatic*/ false));
	}
	for (auto ArgumentPair : Context.Arguments)
	{
		FunctionNames.Add(ArgumentPair.Key);
	}
}
