// Copyright GameDevWare, Denis Zykov 2025

#include "GameData/Formulas/Expressions/FTypeOfExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/IFormulaType.h"

FTypeOfExpression::FTypeOfExpression(const TSharedRef<FJsonObject>& ExpressionObj):
	TypeRef(FExpressionBuildHelper::GetTypeRef(ExpressionObj, FFormulaNotation::TYPE_ATTRIBUTE))
{}

FTypeOfExpression::FTypeOfExpression(const TSharedPtr<FFormulaTypeReference>& TypeRef) :
	TypeRef(TypeRef)
{
}

FFormulaExecutionResult FTypeOfExpression::Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const
{
	if (!this->IsValid())
	{
		return FFormulaExecutionError::ExpressionIsInvalid();
	}
	
	const auto ToType = Context.TypeResolver->FindType(this->TypeRef);
	if (!ToType.IsValid())
	{
		return FFormulaExecutionError::UnableToResolveType(this->TypeRef->GetFullName(/* include generics */ true));
	}
	
	return MakeShared<FFormulaValue>(static_cast<UObject*>(ToType->GetTypeClassOrStruct()));
}

bool FTypeOfExpression::IsValid() const
{
	return this->TypeRef.IsValid();
}

void FTypeOfExpression::DebugPrintTo(FString& OutValue) const
{
	OutValue.Append("typeof(");
	if (this->TypeRef.IsValid())
	{
		OutValue.Append(this->TypeRef->GetFullName(true));
	}
	else
	{
		OutValue.Append(TEXT("#INVALID#"));
	}
	OutValue.Append(") ");
}
