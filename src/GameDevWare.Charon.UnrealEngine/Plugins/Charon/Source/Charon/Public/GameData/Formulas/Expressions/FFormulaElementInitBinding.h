// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "GameData/Formulas/EFormulaExpressionType.h"
#include "GameData/Formulas/FFormulaExecutionContext.h"
#include "GameData/Formulas/FFormulaExecutionResult.h"

class FFormulaExpression;

class CHARON_API FFormulaElementInitBinding : public TSharedFromThis<FFormulaElementInitBinding>
{
public:
	const TArray<TSharedPtr<FFormulaExpression>> Initializers;
	
	explicit FFormulaElementInitBinding(const TSharedRef<FJsonObject>& ExpressionObj);
	explicit FFormulaElementInitBinding(const TArray<TSharedPtr<FFormulaExpression>>& Initializers);
	virtual ~FFormulaElementInitBinding() = default;

	inline static EFormulaExpressionType Type = EFormulaExpressionType::ElementInitBinding;
	virtual EFormulaExpressionType GetType() const  { return Type; }
	virtual bool IsValid() const;
	
	virtual FFormulaExecutionResult Apply(const TSharedRef<FFormulaValue>& Target, const FFormulaExecutionContext& Context) const;
	static void CompleteCollectionInitialization(const TSharedRef<FFormulaValue>& Target);
	
	virtual void DebugPrintTo(FString& OutValue) const;
};

