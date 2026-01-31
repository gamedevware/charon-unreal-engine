// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "GameData/Formulas/EFormulaExpressionType.h"
#include "GameData/Formulas/FFormulaExecutionResult.h"
#include "GameData/Formulas/FFormulaExecutionContext.h"

class FFormulaProperty;

class CHARON_API FFormulaMemberBinding : public TSharedFromThis<FFormulaMemberBinding>
{
public:
	FString const RawMemberName;
	FString const MemberName;
	
	explicit  FFormulaMemberBinding(const FString& RawMemberName);
	virtual ~FFormulaMemberBinding() = default;

	virtual EFormulaExpressionType GetType() const = 0;
	virtual bool IsValid() const = 0;
	
	FFormulaExecutionResult Apply(const TSharedRef<FFormulaValue>& Target, const FFormulaExecutionContext& Context) const;
	virtual FFormulaExecutionResult ApplyToMember(const TSharedRef<FFormulaValue>& Target, const FFormulaProperty* Member, const FFormulaExecutionContext& Context) const = 0;

	FString ToString() const
	{
		FString Builder;
		this->DebugPrintTo(Builder);
		return Builder;
	}
	virtual void DebugPrintTo(FString& OutValue) const = 0;

protected:
	static FString GetExpressionRawMemberName(const TSharedRef<FJsonObject>& ExpressionObj);
	static FString GetMemberName(FString RawMemberName);
};

template <typename BindingT>
BindingT* CastBinding(FFormulaMemberBinding* Binding)
{
	// 1. Ensure Expression isn't null
	// 2. Compare instance type to the template's static Type
	if (Binding && Binding->GetType() == BindingT::Type)
	{
		return static_cast<BindingT*>(Binding);
	}
	return nullptr;
}