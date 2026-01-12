#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaExpression.h"
#include "GameData/Formulas/FFormulaTypeReference.h"
#include "GameData/Formulas/FBinaryExpression.h"
#include "GameData/Formulas/FConditionExpression.h"
#include "GameData/Formulas/FConstantExpression.h"
#include "GameData/Formulas/FConvertExpression.h"
#include "GameData/Formulas/FDefaultExpression.h"
#include "GameData/Formulas/FFormulaConstants.h"
#include "GameData/Formulas/FIndexExpression.h"
#include "GameData/Formulas/FInvokeExpression.h"
#include "GameData/Formulas/FLambdaExpression.h"
#include "GameData/Formulas/FMemberExpression.h"
#include "GameData/Formulas/FNewArrayBoundExpression.h"
#include "GameData/Formulas/FNewExpression.h"
#include "GameData/Formulas/FTypeIsExpression.h"
#include "GameData/Formulas/FTypeOfExpression.h"
#include "GameData/Formulas/FUnaryExpression.h"

DEFINE_LOG_CATEGORY(LogExpressionBuildHelper);

TSharedPtr<FFormulaExpression> FExpressionBuildHelper::CreateExpression(const TSharedPtr<FJsonObject>* ExpressionObj)
{
	FString ExpressionType = GetString(*ExpressionObj, FormulaConstants::EXPRESSION_TYPE_ATTRIBUTE, false);
	if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_PROPERTY_OR_FIELD ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_MEMBER_RESOLVE)
	{
		return MakeShared<FMemberExpression>(ExpressionObj->ToSharedRef());
	}
	else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_INVOKE)
	{
		return MakeShared<FInvokeExpression>(ExpressionObj->ToSharedRef());
	}
	else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_LAMBDA)
	{
		return MakeShared<FLambdaExpression>(ExpressionObj->ToSharedRef());
	}
	else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_INDEX)
	{
		return MakeShared<FIndexExpression>(ExpressionObj->ToSharedRef());
	}
	else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_TYPE_OF)
	{
		return MakeShared<FTypeOfExpression>(ExpressionObj->ToSharedRef());
	}
	else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_CONSTANT)
	{
		return MakeShared<FConstantExpression>(ExpressionObj->ToSharedRef());
	}
	else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_DEFAULT)
	{
		return MakeShared<FDefaultExpression>(ExpressionObj->ToSharedRef());
	}
	else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_NEW)
	{
		return MakeShared<FNewExpression>(ExpressionObj->ToSharedRef());
	}
	else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_NEW_ARRAY_BOUNDS)
	{
		return MakeShared<FNewArrayBoundExpression>(ExpressionObj->ToSharedRef());
	}
	else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_ADD ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_ADD_CHECKED ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_SUBTRACT ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_SUBTRACT_CHECKED ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_LEFT_SHIFT ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_RIGHT_SHIFT ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_GREATER_THAN ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_GREATER_THAN_OR_EQUAL ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_LESS_THAN ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_LESS_THAN_OR_EQUAL ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_POWER ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_DIVIDE ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_DIVIDE_CHECKED ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_MULTIPLY ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_MULTIPLY_CHECKED ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_MODULO ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_EQUAL ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_NOT_EQUAL ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_AND ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_OR ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_EXCLUSIVE_OR ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_AND_ALSO ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_OR_ELSE ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_COALESCE)
	{
		return MakeShared<FBinaryExpression>(ExpressionObj->ToSharedRef());
	}
	else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_NEGATE ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_NEGATE_CHECKED ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_NOT ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_COMPLEMENT ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_GROUP ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_UNCHECKED_SCOPE ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_CHECKED_SCOPE ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_UNARY_PLUS)
	{
		return MakeShared<FUnaryExpression>(ExpressionObj->ToSharedRef());
	}
	else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_TYPE_IS)
	{
		return MakeShared<FTypeIsExpression>(ExpressionObj->ToSharedRef());
	}
	else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_CONVERT ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_CONVERT_CHECKED ||
		ExpressionType == FormulaConstants::EXPRESSION_TYPE_TYPE_AS)
	{
		return MakeShared<FConvertExpression>(ExpressionObj->ToSharedRef());
	}
	else if (ExpressionType == FormulaConstants::EXPRESSION_TYPE_CONDITION)
	{
		return MakeShared<FConditionExpression>(ExpressionObj->ToSharedRef());
	}
	else
	{
		UE_LOG(LogExpressionBuildHelper, Error, TEXT("Unknown expression type: %s"), *ExpressionType);
	}
	return nullptr;
}

TSharedPtr<FFormulaExpression> FExpressionBuildHelper::GetExpression(const TSharedPtr<FJsonObject>& ExpressionObj,
                                                                     const FString& PropertyName, bool bOptional)
{
	if (!ExpressionObj.IsValid()) return nullptr;

	const TSharedPtr<FJsonObject>* PropertyValueObj;
	if (ExpressionObj->TryGetObjectField(PropertyName, PropertyValueObj))
	{
		return CreateExpression(PropertyValueObj);
	}

	if (bOptional)
	{
		return nullptr;
	}

	UE_LOG(LogExpressionBuildHelper, Error, TEXT("Missing required attribute in expression: %s"), *PropertyName);
	return nullptr;
}

FString FExpressionBuildHelper::GetString(const TSharedPtr<FJsonObject>& ExpressionObj, const FString& PropertyName,
                                          bool bOptional)
{
	FString OutString;
	if (ExpressionObj.IsValid() && ExpressionObj->TryGetStringField(PropertyName, OutString))
	{
		return OutString;
	}

	if (bOptional)
	{
		return FString();
	}

	UE_LOG(LogExpressionBuildHelper, Error, TEXT("Missing required string attribute: %s"), *PropertyName);
	return FString();
}

TSharedPtr<FFormulaTypeReference> FExpressionBuildHelper::GetTypeRef(const TSharedPtr<FJsonObject>& ExpressionObj,
                                                                     const FString& PropertyName, bool bOptional)
{
	if (!ExpressionObj.IsValid()) return nullptr;

	// Check if it's a plain string (Type Name)
	FString TypeName;
	if (ExpressionObj->TryGetStringField(PropertyName, TypeName))
	{
		return MakeShared<FFormulaTypeReference>(TypeName);
	}

	// Check if it's a nested object (Complex Type Ref)
	const TSharedPtr<FJsonObject>* TypeObj;
	if (ExpressionObj->TryGetObjectField(PropertyName, TypeObj) && TypeObj->IsValid())
	{
		return MakeShared<FFormulaTypeReference>(TypeObj->ToSharedRef());
	}

	if (bOptional) return nullptr;

	UE_LOG(LogExpressionBuildHelper, Error, TEXT("Missing required TypeRef: %s"), *PropertyName);
	return nullptr;
}

TMap<FString, TSharedPtr<FFormulaExpression>> FExpressionBuildHelper::GetArguments(
	const TSharedPtr<FJsonObject>& ExpressionObj,
	const FString& PropertyName)
{
	TMap<FString, TSharedPtr<FFormulaExpression>> Arguments;
	const TSharedPtr<FJsonObject>* ArgsObj;

	if (ExpressionObj.IsValid() && ExpressionObj->TryGetObjectField(PropertyName, ArgsObj))
	{
		for (auto& Pair : (*ArgsObj)->Values)
		{
			Arguments.Add(Pair.Key, GetExpression(*ArgsObj, Pair.Key, false));
		}
	}

	return Arguments;
}

TArray<TSharedPtr<FFormulaTypeReference>> FExpressionBuildHelper::GetTypeRefArguments(
	const TSharedPtr<FJsonObject>& ExpressionObj, const FString& PropertyName)
{
	TArray<TSharedPtr<FFormulaTypeReference>> Arguments;
	const TSharedPtr<FJsonObject>* ArgsObj;

	if (ExpressionObj.IsValid() && ExpressionObj->TryGetObjectField(PropertyName, ArgsObj))
	{
		int32 Index = 0;
		while (true)
		{
			FString IndexStr = FString::FromInt(Index);
			if (!(*ArgsObj)->HasField(IndexStr)) break;

			Arguments.Add(GetTypeRef(*ArgsObj, IndexStr, false));
			Index++;
		}
	}

	return Arguments;
}
