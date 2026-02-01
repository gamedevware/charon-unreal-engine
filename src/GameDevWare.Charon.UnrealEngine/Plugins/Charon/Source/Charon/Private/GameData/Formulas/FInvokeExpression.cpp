// Copyright GameDevWare, Denis Zykov 2025

#include "GameData/Formulas/Expressions/FInvokeExpression.h"
#include "GameData/Formulas/Expressions/FFormulaExpression.h"
#include "GameData/Formulas/Expressions/FMemberExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaFunction.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/IFormulaType.h"
#include "GameData/Formulas/FFormulaTypeReference.h"

DEFINE_LOG_CATEGORY(LogInvokeExpression);

FInvokeExpression::FInvokeExpression(const TSharedRef<FJsonObject>& ExpressionObj):
	Arguments(FExpressionBuildHelper::GetArguments(ExpressionObj, FFormulaNotation::ARGUMENTS_ATTRIBUTE)),
	Expression(FExpressionBuildHelper::GetExpression(ExpressionObj, FFormulaNotation::EXPRESSION_ATTRIBUTE))
{
}

FInvokeExpression::FInvokeExpression(const TSharedPtr<FFormulaExpression>& Expression,
	const TMap<FString, TSharedPtr<FFormulaExpression>>& Arguments) : Arguments(Arguments), Expression(Expression)
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

FFormulaExecutionResult FInvokeExpression::Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const
{
	if (!this->IsValid())
	{
		return FFormulaExecutionError::ExpressionIsInvalid();
	}
	
	TArray<UField*> TypeArguments;
	FString MemberName;
	TSharedPtr<FFormulaTypeReference> StaticTypeReference = nullptr;
	TSharedPtr<IFormulaType> StaticTypeDescription = nullptr;
	const FFormulaFunction* FormulaFunction = nullptr;
	const FMemberExpression* MemberExpression = CastExpression<FMemberExpression>(this->Expression.Get());

	// resolve type arguments
	for (auto TypeArgumentReference : MemberExpression->TypeArguments)
	{
		const auto TypeArgument = Context.TypeResolver->FindType(TypeArgumentReference);
		if (!TypeArgument.IsValid() || !TypeArgument->GetTypeClassOrStruct())
		{
			return FFormulaExecutionError::UnableToResolveType(TypeArgumentReference->GetFullName(/* include generics */ true));
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
		MemberName.Reset();
		MemberName.Append(TEXT("Invoke"));
		
		return FindAndInvokeFunction(Result.GetValue(), MemberName, &TypeArguments, Context);
	}
	else if (MemberExpression && MemberExpression->TryGetTypeReferenceAndMemberName(StaticTypeReference, MemberName) &&
		(StaticTypeDescription = Context.TypeResolver->FindType(StaticTypeReference)).IsValid() &&
		StaticTypeDescription->TryGetFunction(MemberName, /* bStatic */ true, FormulaFunction))
	{
		// ex. System.Math.Add()
		return InvokeFunction(FFormulaValue::Null(), FormulaFunction, MemberName, &TypeArguments, Context);
	}
	else if (MemberExpression && MemberExpression->Expression.IsValid())
	{
		// ex. value.Add(1, 2)
		auto Result = this->ExecuteExpression(MemberExpression->Expression.ToSharedRef(), Context);
		if (Result.HasError())
		{
			return Result; // propagate error
		}
		return FindAndInvokeFunction(Result.GetValue(), MemberExpression->MemberName, &TypeArguments, Context);
	}
	else if (MemberExpression && !Context.Global->IsNull())
	{
		// ex. hello()
		return FindAndInvokeFunction(Context.Global, MemberExpression->MemberName, &TypeArguments, Context);
	}
	else if (StaticTypeDescription.IsValid())
	{
		const FString AllStaticMemberNames = FString::Join(StaticTypeDescription->GetFunctionNames(/* bStatic */ true), TEXT(", "));
		return FFormulaExecutionError::CantFindStaticMember(StaticTypeDescription->GetCPPType(), MemberName, AllStaticMemberNames);
	}
	else
	{
		const TSet<FString> AllGlobalFunctionNames;
		GetGlobalFunctionNames(AllGlobalFunctionNames, Context);
		return FFormulaExecutionError::UnableToResolveGlobalName(MemberName, FString::Join(AllGlobalFunctionNames, TEXT(", ")));
	}
}

bool FInvokeExpression::IsValid() const
{
	if (!this->Expression.IsValid())
	{
		return false;
	}
	for (auto ArgumentPair : this->Arguments)
	{
		if (!ArgumentPair.Value.IsValid())
		{
			return false;
		}
	}
	return true;
}

void FInvokeExpression::DebugPrintTo(FString& OutValue) const
{
	if (this->Expression.IsValid())
	{
		if (this->Expression->GetType() != EFormulaExpressionType::MemberExpression)
		{
			OutValue.Append("(");
		}
		this->Expression->DebugPrintTo(OutValue);
		
		if (this->Expression->GetType() != EFormulaExpressionType::MemberExpression)
		{
			OutValue.Append(")");
		}
	}
	else
	{
		OutValue.Append(TEXT("#INVALID#"));
	}
	OutValue.Append("(");
	bool bFirstArgument = true;;
	for (auto ArgumentPair : this->Arguments)
	{
		if (!bFirstArgument)
		{
			OutValue.Append(", ");
		}
		bFirstArgument = false;

		const bool bIsNumberKey = ArgumentPair.Key == TEXT("0") || FCString::Atoi(*ArgumentPair.Key) != 0;
		if (!bIsNumberKey)
		{
			OutValue.Append(ArgumentPair.Key).Append(": ");
		}
		if (ArgumentPair.Value.IsValid())
		{
			ArgumentPair.Value->DebugPrintTo(OutValue);
		}
		else
		{
			OutValue.Append(TEXT("#INVALID#"));
		}
	}
	OutValue.Append(")");
}

FFormulaExecutionResult FInvokeExpression::ExecuteExpression(const TSharedRef<FFormulaExpression>& TargetExpression, const FFormulaExecutionContext& Context) const
{
	const bool IsNullPropagation = this->IsNullPropagationEnabled() || Context.AutoNullPropagation;
	const auto TargetResult = TargetExpression->Execute(Context, nullptr);
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
			return FFormulaExecutionError::NullReference();
		}
	}
	return TargetResult; 
}

FFormulaExecutionResult FInvokeExpression::FindAndInvokeFunction(const TSharedRef<FFormulaValue>& Target, const FString& FunctionName, const TArray<UField*>* TypeArguments, const FFormulaExecutionContext& Context) const
{
	if (Target->IsNull())
	{
		if (this->IsNullPropagationEnabled())
		{
			return FFormulaValue::Null();
		}
		else
		{
			return FFormulaExecutionError::NullReference();
		}
	}
	
	const FFormulaFunction* FormulaFunction = nullptr;
	const auto TargetTypeDescription = Context.TypeResolver->GetType(Target);
	if (TargetTypeDescription->TryGetFunction(FunctionName, /* bStatic */ false, FormulaFunction))
	{
		check(FormulaFunction);

		return InvokeFunction(Target, FormulaFunction, FunctionName, TypeArguments, Context);
	}

	FString AllFunctionNames =  FString::Join(TargetTypeDescription->GetFunctionNames(/* bStatic */ false),TEXT(", "));
	return FFormulaExecutionError::CantFindMember(Target->GetCPPType(), FunctionName, AllFunctionNames);
}

FFormulaExecutionResult FInvokeExpression::InvokeFunction(const TSharedRef<FFormulaValue>& Target, const FFormulaFunction* FormulaFunction, const FString& FunctionName, const TArray<UField*>* TypeArguments, const FFormulaExecutionContext& Context) const
{
	check(FormulaFunction);
	
	FFormulaInvokeArguments CallArguments;
	for (auto ArgumentPair : this->Arguments)
	{
		const TSharedPtr<FFormulaExpression>& ArgumentExpression = ArgumentPair.Value;
		const FString& ArgumentIndexOrName = ArgumentPair.Key;
		FProperty* ArgumentType = FindArgumentType(FormulaFunction, ArgumentIndexOrName);
		const auto ArgumentResult = ArgumentExpression->Execute(Context, ArgumentType);
		if (ArgumentResult.HasError())
		{
			return ArgumentResult;
		}
		const auto ArgumentValue = ArgumentResult.GetValue();
		CallArguments.AddArgument(ArgumentPair.Key, ArgumentValue, FFormulaInvokeArguments::GetArgumentFlags(ArgumentExpression, ArgumentValue, Context));
	}

	TSharedPtr<FFormulaValue> InvokeValue;
	if (FormulaFunction->TryInvoke(Target, CallArguments, nullptr, TypeArguments, InvokeValue))
	{
		// copy out parameters back to Context
		for (auto UpdatedOutArgument : CallArguments.GetUpdatedOutArguments())
		{
			if (const FString* ContextArgumentName = Context.Arguments.FindKey(UpdatedOutArgument.Key))
			{
				Context.Arguments.Add(*ContextArgumentName, UpdatedOutArgument.Value);
			}
		}
		//
		
		return InvokeValue;
	}

	// binding failed
	TArray<FString> ParameterTypes;
	CallArguments.GetParameterTypes(ParameterTypes);
	return FFormulaExecutionError::UnableToBindMethodToParameters(Target->GetCPPType(), FunctionName, FString::Join(ParameterTypes, TEXT(", ")), CallArguments.Num());
}

FProperty* FInvokeExpression::FindArgumentType(const FFormulaFunction* FormulaFunction,	const FString& ArgumentIndexOrName)
{
	FString ParameterNameOrIndex;
	for (int i = 0; i < FormulaFunction->Parameters.Num(); ++i)
	{	
		const FProperty* Parameter = FormulaFunction->Parameters[i]; 
		
		// parameter index match
		ParameterNameOrIndex.Reset();
		ParameterNameOrIndex.AppendInt(i);
		if (ArgumentIndexOrName == ParameterNameOrIndex)
		{
			return const_cast<FProperty*>(Parameter);
		}
		
		// parameter name match
		ParameterNameOrIndex.Reset();
		Parameter->GetName(ParameterNameOrIndex);
		if (ArgumentIndexOrName == ParameterNameOrIndex)
		{
			return const_cast<FProperty*>(Parameter);
		}
	}
	return nullptr;
}

void FInvokeExpression::GetGlobalFunctionNames(TSet<FString> FunctionNames, const FFormulaExecutionContext& Context)
{
	if (!Context.Global->IsNull())
	{
		const auto GlobalType = Context.TypeResolver->GetType(Context.Global);
		FunctionNames.Append(GlobalType->GetFunctionNames(/*bStatic*/ false));
	}
	for (auto ArgumentPair : Context.Arguments)
	{
		FunctionNames.Add(ArgumentPair.Key);
	}
}
