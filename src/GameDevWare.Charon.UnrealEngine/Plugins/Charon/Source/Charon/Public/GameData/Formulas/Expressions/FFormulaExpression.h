// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "../EFormulaExpressionType.h"
#include "../FFormulaExecutionContext.h"
#include "../FFormulaExecutionResult.h"
#include "../FFormulaValue.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Dom/JsonObject.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "Templates/SharedPointer.h"

class FFormulaMemberBinding;

class CHARON_API FFormulaExpression : public TSharedFromThis<FFormulaExpression>
{
public:
	virtual ~FFormulaExpression() = default;

	virtual EFormulaExpressionType GetType() const = 0;
	virtual bool IsNullPropagationEnabled() const { return false; }
	virtual bool IsValid() const = 0;
	
	virtual FFormulaExecutionResult Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const
	{
		return FFormulaValue::Null();
	}

	FString ToString() const
	{
		FString Builder;
		this->DebugPrintTo(Builder);
		return Builder;
	}
	virtual void DebugPrintTo(FString& OutValue) const = 0;
	
protected:
	static FString GetExpressionRawMemberName(const TSharedRef<FJsonObject>& ExpressionObj)
	{
		// Equivalent to ExpressionBuildHelper.GetString logic
		FString MemberName;
		if (!ExpressionObj->TryGetStringField(FFormulaNotation::PROPERTY_OR_FIELD_NAME_ATTRIBUTE, MemberName))
		{
			ExpressionObj->TryGetStringField(FFormulaNotation::NAME_ATTRIBUTE, MemberName);
		}
		return MemberName;
	}
	static FString GetMemberName(FString RawMemberName)
	{
		return RawMemberName.Len() > 0 && RawMemberName[0] == '@' ? RawMemberName.RightChop(1) : RawMemberName;
	}
};

template <typename ExpressionT>
ExpressionT* CastExpression(FFormulaExpression* Expression)
{
	// 1. Ensure Expression isn't null
	// 2. Compare instance type to the template's static Type
	if (Expression && Expression->GetType() == ExpressionT::Type)
	{
		return static_cast<ExpressionT*>(Expression);
	}
	return nullptr;
}

template <typename ExpressionT>
TSharedPtr<ExpressionT> CastExpression(TSharedPtr<FFormulaExpression> Expression)
{
	// 1. Ensure Expression isn't null
	// 2. Compare instance type to the template's static Type
	if (Expression && Expression->GetType() == ExpressionT::Type)
	{
		return StaticCastSharedPtr<ExpressionT>(Expression);
	}
	return nullptr;
}