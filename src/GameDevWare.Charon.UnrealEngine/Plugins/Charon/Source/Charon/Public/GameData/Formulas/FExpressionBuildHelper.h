#pragma once

class FFormulaTypeReference;
class FFormulaExpression;

DECLARE_LOG_CATEGORY_EXTERN(LogExpressionBuildHelper, Log, All);

class CHARON_API FExpressionBuildHelper
{
public:
	static TSharedPtr<FFormulaExpression> CreateExpression(const TSharedPtr<FJsonObject>* ExpressionObj);
	static TSharedPtr<FFormulaExpression> GetExpression(const TSharedPtr<FJsonObject>& ExpressionObj, const FString& PropertyName, bool bOptional = false);
	static FString GetString(const TSharedPtr<FJsonObject>& ExpressionObj, const FString& PropertyName, bool bOptional = false);
	static TSharedPtr<FFormulaTypeReference> GetTypeRef(const TSharedPtr<FJsonObject>& ExpressionObj, const FString& PropertyName, bool bOptional = false);
	static TMap<FString, TSharedPtr<FFormulaExpression>> GetArguments(const TSharedPtr<FJsonObject>& ExpressionObj, const FString& PropertyName);
	static TArray<TSharedPtr<FFormulaTypeReference>> GetTypeRefArguments(const TSharedPtr<FJsonObject>& ExpressionObj, const FString& PropertyName);
};
