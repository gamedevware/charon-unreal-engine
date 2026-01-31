// Copyright GameDevWare, Denis Zykov 2025

#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/Expressions/FFormulaExpression.h"
#include "GameData/Formulas/FFormulaTypeReference.h"
#include "GameData/Formulas/Expressions/FBinaryExpression.h"
#include "GameData/Formulas/Expressions/FConditionExpression.h"
#include "GameData/Formulas/Expressions/FConstantExpression.h"
#include "GameData/Formulas/Expressions/FConvertExpression.h"
#include "GameData/Formulas/Expressions/FDefaultExpression.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/Expressions/FFormulaMemberAssignmentBinding.h"
#include "GameData/Formulas/Expressions/FFormulaMemberListBinding.h"
#include "GameData/Formulas/Expressions/FFormulaMemberMemberBinding.h"
#include "GameData/Formulas/Expressions/FFormulaElementInitBinding.h"
#include "GameData/Formulas/Expressions/FIndexExpression.h"
#include "GameData/Formulas/Expressions/FInvokeExpression.h"
#include "GameData/Formulas/Expressions/FLambdaExpression.h"
#include "GameData/Formulas/Expressions/FMemberExpression.h"
#include "GameData/Formulas/Expressions/FMemberInitExpression.h"
#include "GameData/Formulas/Expressions/FNewArrayBoundExpression.h"
#include "GameData/Formulas/Expressions/FNewArrayInitExpression.h"
#include "GameData/Formulas/Expressions/FNewExpression.h"
#include "GameData/Formulas/Expressions/FTypeIsExpression.h"
#include "GameData/Formulas/Expressions/FTypeOfExpression.h"
#include "GameData/Formulas/Expressions/FUnaryExpression.h"

DEFINE_LOG_CATEGORY(LogExpressionBuildHelper);

TSharedPtr<FFormulaExpression> FExpressionBuildHelper::CreateExpression(const TSharedPtr<FJsonObject>* ExpressionObj)
{
	check(ExpressionObj->IsValid());

	const FString ExpressionType = GetString(*ExpressionObj, FFormulaNotation::EXPRESSION_TYPE_ATTRIBUTE, false);
	if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_PROPERTY_OR_FIELD ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_MEMBER_RESOLVE)
	{
		return MakeShared<FMemberExpression>(ExpressionObj->ToSharedRef());
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_INVOKE)
	{
		return MakeShared<FInvokeExpression>(ExpressionObj->ToSharedRef());
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_LAMBDA)
	{
		return MakeShared<FLambdaExpression>(ExpressionObj->ToSharedRef());
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_INDEX)
	{
		return MakeShared<FIndexExpression>(ExpressionObj->ToSharedRef());
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_TYPE_OF)
	{
		return MakeShared<FTypeOfExpression>(ExpressionObj->ToSharedRef());
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_CONSTANT)
	{
		return MakeShared<FConstantExpression>(ExpressionObj->ToSharedRef());
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_DEFAULT)
	{
		return MakeShared<FDefaultExpression>(ExpressionObj->ToSharedRef());
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_NEW)
	{
		return MakeShared<FNewExpression>(ExpressionObj->ToSharedRef());
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_NEW_ARRAY_BOUNDS)
	{
		return MakeShared<FNewArrayBoundExpression>(ExpressionObj->ToSharedRef());
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_NEW_ARRAY_INIT)
	{
		return MakeShared<FNewArrayInitExpression>(ExpressionObj->ToSharedRef());
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_MEMBER_INIT)
	{
		return MakeShared<FMemberInitExpression>(ExpressionObj->ToSharedRef());
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_ADD ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_ADD_CHECKED ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_SUBTRACT ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_SUBTRACT_CHECKED ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_LEFT_SHIFT ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_RIGHT_SHIFT ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_GREATER_THAN ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_GREATER_THAN_OR_EQUAL ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_LESS_THAN ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_LESS_THAN_OR_EQUAL ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_POWER ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_DIVIDE ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_DIVIDE_CHECKED ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_MULTIPLY ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_MULTIPLY_CHECKED ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_MODULO ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_EQUAL ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_NOT_EQUAL ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_AND ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_OR ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_EXCLUSIVE_OR ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_AND_ALSO ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_OR_ELSE ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_COALESCE)
	{
		return MakeShared<FBinaryExpression>(ExpressionObj->ToSharedRef());
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_NEGATE ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_NEGATE_CHECKED ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_NOT ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_COMPLEMENT ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_GROUP ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_UNCHECKED_SCOPE ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_CHECKED_SCOPE ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_UNARY_PLUS)
	{
		return MakeShared<FUnaryExpression>(ExpressionObj->ToSharedRef());
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_TYPE_IS)
	{
		return MakeShared<FTypeIsExpression>(ExpressionObj->ToSharedRef());
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_CONVERT ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_CONVERT_CHECKED ||
		ExpressionType == FFormulaNotation::EXPRESSION_TYPE_TYPE_AS)
	{
		return MakeShared<FConvertExpression>(ExpressionObj->ToSharedRef());
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_CONDITION)
	{
		return MakeShared<FConditionExpression>(ExpressionObj->ToSharedRef());
	}
	else
	{
		UE_LOG(LogExpressionBuildHelper, Error, TEXT("Unknown expression type: %s"), *ExpressionType);
	}
	return nullptr;
}

TSharedPtr<FFormulaMemberBinding> FExpressionBuildHelper::CreateBinding(const TSharedPtr<FJsonObject>* BindingObj)
{
	const FString ExpressionType = GetString(*BindingObj, FFormulaNotation::EXPRESSION_TYPE_ATTRIBUTE, false);
	if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_LIST_BINDING)
	{
		return MakeShared<FFormulaMemberListBinding>(BindingObj->ToSharedRef());
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_ASSIGNMENT_BINDING)
	{
		return MakeShared<FFormulaMemberAssignmentBinding>(BindingObj->ToSharedRef());
	}
	else if (ExpressionType == FFormulaNotation::EXPRESSION_TYPE_MEMBER_BINDING)
	{
		return MakeShared<FFormulaMemberMemberBinding>(BindingObj->ToSharedRef());
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

bool FExpressionBuildHelper::GetUseNullPropagation(const TSharedPtr<FJsonObject>& ExpressionObj, bool bOptional)
{
	bool bUseNullPropagation = false;
	if (ExpressionObj->TryGetBoolField(FFormulaNotation::USE_NULL_PROPAGATION_ATTRIBUTE, bUseNullPropagation))
	{
	return bUseNullPropagation;
	}

	if (bOptional)
	{
		return false;
	}
	
	UE_LOG(LogExpressionBuildHelper, Error, TEXT("Missing required attribute in expression: %s"), *FFormulaNotation::USE_NULL_PROPAGATION_ATTRIBUTE);
	return false;
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
			auto CreatedExpression = GetExpression(*ArgsObj, Pair.Key, false);
			Arguments.Add(Pair.Key, CreatedExpression);
		}
	}

	return Arguments;
}

TArray<TSharedPtr<FFormulaExpression>> FExpressionBuildHelper::GetArgumentsList(
	const TSharedPtr<FJsonObject>& ExpressionObj,
	const FString& PropertyName)
{
	
	TArray<TSharedPtr<FFormulaExpression>> Arguments;

	const TSharedPtr<FJsonObject>* ArgumentsObjPtr;
	if (ExpressionObj.IsValid() && ExpressionObj->TryGetObjectField(PropertyName, ArgumentsObjPtr))
	{
		for (auto& ArgumentPair : (*ArgumentsObjPtr)->Values)
		{
			auto CreatedExpression = GetExpression(*ArgumentsObjPtr, ArgumentPair.Key, false);
			Arguments.Add(CreatedExpression);
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* ArgumentsArrayPtr;
	if (ExpressionObj.IsValid() && ExpressionObj->TryGetArrayField(PropertyName, ArgumentsArrayPtr))
	{
		for (auto& Argument : *ArgumentsArrayPtr)
		{
			const TSharedPtr<FJsonObject>* ArgumentObj;
			if (!Argument->TryGetObject(ArgumentObj))
			{
				Arguments.Add(nullptr);
				continue; // invalid object
			}
			auto CreatedExpression = CreateExpression(ArgumentObj);
			Arguments.Add(CreatedExpression);
		}
	}

	return Arguments;
}

TArray<TSharedPtr<FFormulaTypeReference>> FExpressionBuildHelper::GetTypeRefArguments(
	const TSharedPtr<FJsonObject>& ExpressionObj, const FString& PropertyName)
{
	TArray<TSharedPtr<FFormulaTypeReference>> Arguments;
	const TSharedPtr<FJsonObject>* ArgumentsObjPtr;

	if (ExpressionObj.IsValid() && ExpressionObj->TryGetObjectField(PropertyName, ArgumentsObjPtr))
	{
		int32 Index = 0;
		while (true)
		{
			FString IndexStr = FString::FromInt(Index);
			if (!(*ArgumentsObjPtr)->HasField(IndexStr)) break;

			auto TypeReference = GetTypeRef(*ArgumentsObjPtr, IndexStr, /* bOptional */ false);
			Arguments.Add(TypeReference);
			Index++;
		}
	}

	return Arguments;
}

TArray<TSharedPtr<FFormulaMemberBinding>> FExpressionBuildHelper::GetBindings(
	const TSharedPtr<FJsonObject>& ExpressionObj, const FString& PropertyName)
{
	TArray<TSharedPtr<FFormulaMemberBinding>> Bindings;
	
	const TArray<TSharedPtr<FJsonValue>>* BindingsArrayPtr;
	if (ExpressionObj.IsValid() && ExpressionObj->TryGetArrayField(PropertyName, BindingsArrayPtr))
	{
			
		for (auto& BindingValue : *BindingsArrayPtr)
		{
			const TSharedPtr<FJsonObject>* BindingObjPtr;
			if (!BindingValue->TryGetObject(BindingObjPtr))
			{
				Bindings.Add(nullptr);
				continue; // invalid object
			}
			
			auto Binding = CreateBinding(BindingObjPtr);
			Bindings.Add(Binding);
		}
	}
	return Bindings;
}

TArray<TSharedPtr<FFormulaElementInitBinding>> FExpressionBuildHelper::GetElementInitBindings(const TSharedPtr<FJsonObject>& ExpressionObj, const FString& PropertyName)
{
	TArray<TSharedPtr<FFormulaElementInitBinding>> Bindings;
	
	const TArray<TSharedPtr<FJsonValue>>* BindingsArrayPtr;
	if (ExpressionObj.IsValid() && ExpressionObj->TryGetArrayField(PropertyName, BindingsArrayPtr))
	{
		for (auto& BindingValue : *BindingsArrayPtr)
		{
			const TSharedPtr<FJsonObject>* BindingObjPtr;
			if (!BindingValue->TryGetObject(BindingObjPtr))
			{
				Bindings.Add(nullptr);
				continue; // invalid object
			}
			
			const FString ExpressionType = GetString(*BindingObjPtr, FFormulaNotation::EXPRESSION_TYPE_ATTRIBUTE, /*optional*/ true);
			if (ExpressionType.IsEmpty() || ExpressionType == FFormulaNotation::EXPRESSION_TYPE_ELEMENT_INIT_BINDING)
			{
				auto Binding = MakeShared<FFormulaElementInitBinding>(BindingObjPtr->ToSharedRef());
				Bindings.Add(Binding);
			}
			else
			{
				UE_LOG(LogExpressionBuildHelper, Error, TEXT("Unknown expression type: %s"), *ExpressionType);
			}
		}
	}
	return Bindings;
}