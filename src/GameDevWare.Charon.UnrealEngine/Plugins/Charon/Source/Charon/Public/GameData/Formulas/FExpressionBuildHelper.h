// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Dom/JsonObject.h"
#include "Templates/SharedPointer.h"
#include "Misc/EngineVersionComparison.h"

class FFormulaTypeReference;
class FFormulaExpression;
class FFormulaMemberBinding;
class FFormulaElementInitBinding;

DECLARE_LOG_CATEGORY_EXTERN(LogExpressionBuildHelper, Log, All);

class CHARON_API FExpressionBuildHelper
{
#if UE_VERSION_NEWER_THAN(5, 4, -1)
	using FJsonKeyString = FStringView;
#else
	using FJsonKeyString = FString;
#endif
	
public:
	static TSharedPtr<FFormulaExpression> CreateExpression(const TSharedPtr<FJsonObject>* ExpressionObj);
	static TSharedPtr<FFormulaMemberBinding> CreateBinding(const TSharedPtr<FJsonObject>* BindingObj);
	
	static TSharedPtr<FFormulaExpression> GetExpression(const TSharedPtr<FJsonObject>& ExpressionObj, const FJsonKeyString& PropertyName, bool bOptional = false);
	static bool GetUseNullPropagation(const TSharedPtr<FJsonObject>& ExpressionObj, bool bOptional = true);
	static FString GetString(const TSharedPtr<FJsonObject>& ExpressionObj, const FJsonKeyString& PropertyName, bool bOptional = false);
	static TSharedPtr<FFormulaTypeReference> GetTypeRef(const TSharedPtr<FJsonObject>& ExpressionObj, const FJsonKeyString& PropertyName, bool bOptional = false);
	static TMap<FString, TSharedPtr<FFormulaExpression>> GetArguments(const TSharedPtr<FJsonObject>& ExpressionObj, const FJsonKeyString& PropertyName);
	static TArray<TSharedPtr<FFormulaExpression>> GetArgumentsList(const TSharedPtr<FJsonObject>& ExpressionObj, const FJsonKeyString& PropertyName);
	static TArray<TSharedPtr<FFormulaTypeReference>> GetTypeRefArguments(const TSharedPtr<FJsonObject>& ExpressionObj, const FJsonKeyString& PropertyName);
	static TArray<TSharedPtr<FFormulaMemberBinding>> GetBindings(const TSharedPtr<FJsonObject>& ExpressionObj, const FJsonKeyString& PropertyName);
	static TArray<TSharedPtr<FFormulaElementInitBinding>> GetElementInitBindings(const TSharedPtr<FJsonObject>& ExpressionObj, const FJsonKeyString& PropertyName);
};
