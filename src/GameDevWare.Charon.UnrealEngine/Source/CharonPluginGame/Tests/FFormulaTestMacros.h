// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "limits"
#include "GameData/Formulas/Expressions/FDefaultExpression.h"
#include "JsonUtils/JsonConversion.h"
#include "Tests/TestHarnessAdapter.h"

#define REQUIRE_NO_ERROR() \
	CHECK_FALSE_MESSAGE(Result.GetError().Message, Result.HasError()) \
	REQUIRE(!Result.HasError()) 

#define REQUIRE_ERROR(ERROR_CODE) \
	CHECK_MESSAGE(TEXT("Expression should fail"), Result.HasError()) \
	REQUIRE(Result.HasError()) \
	if (Result.GetError().Code != ERROR_CODE) { WARN(Result.GetError().Message); } \
	CHECK_EQUALS(TEXT("Error code"), Result.GetError().Code, ERROR_CODE)

#define CHECK_VALUE(EXPECTED) \
	CHECK_MESSAGE(TEXT("Unexpected Value type: " + Result.GetValue()->GetCPPType()), IsValueSameType<decltype(EXPECTED)>(Result)) \
	CHECK_EQUALS(TEXT("Value"), GetValue<decltype(EXPECTED)>(Result), EXPECTED) 

#define MAKE_ARRAY_TYPE_REF(ELEMENT_TYPE) MakeShared<FFormulaTypeReference>(FString(TEXT("Array")), TArray<TSharedPtr<FFormulaTypeReference>> { MakeShared<FFormulaTypeReference>(FString(TEXT(ELEMENT_TYPE))) })
#define MAKE_SET_TYPE_REF(ELEMENT_TYPE) MakeShared<FFormulaTypeReference>(FString(TEXT("Set")), TArray<TSharedPtr<FFormulaTypeReference>> { MakeShared<FFormulaTypeReference>(FString(TEXT(ELEMENT_TYPE))) })
#define MAKE_MAP_TYPE_REF(KEY_TYPE, VALUE_TYPE) MakeShared<FFormulaTypeReference>(FString(TEXT("Map")), TArray<TSharedPtr<FFormulaTypeReference>> { MakeShared<FFormulaTypeReference>(FString(TEXT(KEY_TYPE))), MakeShared<FFormulaTypeReference>(FString(TEXT(VALUE_TYPE))) })
#define EXPR_ELEM_INIT(...) MakeShared<FFormulaElementInitBinding>(TArray<TSharedPtr<FFormulaExpression>> { __VA_ARGS__ })
#define EXPR_CONST_NULL() MakeShared<FConstantExpression>(FFormulaValue::Null(), MakeShared<FFormulaTypeReference>(TEXT("Object")))
#define EXPR_CONST(VALUE) MakeShared<FConstantExpression>(MakeShared<FFormulaValue>(VALUE), MakeShared<FFormulaTypeReference>(FFormulaValue(VALUE).GetCPPType()))
#define EXPR_CONST_OBJECT(VALUE, PROPERTY) MakeShared<FConstantExpression>(MakeShared<FFormulaValue>(PROPERTY, &VALUE), MakeShared<FFormulaTypeReference>(PROPERTY->PropertyClass.GetName()))
#define EXPR_MEMBER(VALUE, MEMBER_NAME, NULL_PROP) Expression = MakeShared<FMemberExpression>(VALUE, FString(TEXT(MEMBER_NAME)), EmptyTypeArguments, NULL_PROP)
#define EXPR_ARG(ARG_NAME) Expression = MakeShared<FMemberExpression>(nullptr, FString(TEXT(ARG_NAME)), EmptyTypeArguments, false)

#define TEST_EXPR_CONST_CHECK_VALUE(VALUE, RESULT_VALUE) \
		Expression = MakeShared<FConstantExpression>( \
			MakeShared<FFormulaValue>(VALUE), \
			MakeShared<FFormulaTypeReference>(FFormulaValue(RESULT_VALUE).GetCPPType()) \
		); \
		INFO("Testing `" + Expression->ToString() + "` expression"); \
		Result = Expression->Execute(Context, nullptr); \
		REQUIRE_NO_ERROR(); \
		CHECK_VALUE(RESULT_VALUE);
// end of TEST_EXPR_CONST_CHECK_VALUE

#define TEST_EXPR_BINARY(V1, V2, OP_T, OP) \
		Expression = MakeShared<FBinaryExpression>( \
			EXPR_CONST(V1), \
			EXPR_CONST(V2), \
			OP_T \
		); \
		INFO("Testing `" + Expression->ToString() + "` expression"); \
		Result = Expression->Execute(Context, nullptr); \
		if constexpr (requires { V1 OP V2; }) \
		{ \
			REQUIRE_NO_ERROR(); \
			CHECK_VALUE(V1 OP V2); \
		} \
		else \
		{ \
			REQUIRE_ERROR(EFormulaExecutionErrorCode::Op_BinaryNotSupported); \
		}
// end of TEST_EXPR_BINARY

#define TEST_EXPR_BINARY_CHECK_VALUE(V1, V2, OP_T, RESULT_VALUE) \
		Expression = MakeShared<FBinaryExpression>( \
			EXPR_CONST(V1), \
			EXPR_CONST(V2), \
			OP_T \
		); \
		INFO("Testing `" + Expression->ToString() + "` expression"); \
		Result = Expression->Execute(Context, nullptr); \
		REQUIRE_NO_ERROR(); \
		CHECK_VALUE(RESULT_VALUE);
// end of TEST_EXPR_BINARY_CHECK_VALUE
		
#define TEST_FAIL_EXPR_BINARY(V1, V2, OP_T, OP) \
		Expression = MakeShared<FBinaryExpression>( \
			EXPR_CONST(V1), \
			EXPR_CONST(V2), \
			OP_T \
		); \
		Result = Expression->Execute(Context, nullptr); \
		INFO("Testing `" + Expression->ToString() + "` expression"); \
		REQUIRE_ERROR(EFormulaExecutionErrorCode::Op_BinaryNotSupported);
// end of TEST_FAIL_EXPR_BINARY

#define TEST_EXPR_CONVERT_CHECK_VALUE(VALUE, RESULT_VALUE, CONV_TYPE) \
		Expression = MakeShared<FConvertExpression>( \
			EXPR_CONST(VALUE), \
			MakeShared<FFormulaTypeReference>(FFormulaValue(RESULT_VALUE).GetCPPType()), \
			CONV_TYPE \
		); \
		INFO("Testing `" + Expression->ToString() + "` expression"); \
		Result = Expression->Execute(Context, nullptr); \
		REQUIRE_NO_ERROR(); \
		CHECK_VALUE(RESULT_VALUE);
// end of TEST_EXPR_CONVERT_CHECK_VALUE

#define TEST_EXPR_CONVERT_TYPED_CHECK_VALUE(VALUE, VALUE_TYPE, RESULT_VALUE, CONV_TYPE) \
		Expression = MakeShared<FConvertExpression>( \
			VALUE, \
			MakeShared<FFormulaTypeReference>(VALUE_TYPE), \
			CONV_TYPE \
		); \
		INFO("Testing `" + Expression->ToString() + "` expression"); \
		Result = Expression->Execute(Context, nullptr); \
		REQUIRE_NO_ERROR(); \
		CHECK_VALUE(RESULT_VALUE);
// end of TEST_EXPR_CONVERT_TYPED_CHECK_VALUE

#define TEST_EXPR_DEFAULT_CHECK_VALUE(RESULT_VALUE) \
		Expression = MakeShared<FDefaultExpression>( \
			MakeShared<FFormulaTypeReference>(FFormulaValue(RESULT_VALUE).GetCPPType()) \
		); \
		INFO("Testing `" + Expression->ToString() + "` expression"); \
		Result = Expression->Execute(Context, nullptr); \
		REQUIRE_NO_ERROR(); \
		CHECK_VALUE(RESULT_VALUE);
// end of TEST_EXPR_DEFAULT_CHECK_VALUE

#define TEST_EXPR_INDEX_CHECK_VALUE(OBJ_VALUE, PATH, INDEX, RESULT, NULL_PROP) \
		Expression = MakeShared<FIndexExpression>( \
			EXPR_MEMBER(OBJ_VALUE, PATH, NULL_PROP), \
			TMap<FString, TSharedPtr<FFormulaExpression>> { \
				{ FString(TEXT("0")), EXPR_CONST(INDEX) } \
			}, \
			NULL_PROP \
		); \
		INFO("Testing `" + Expression->ToString() + "` expression"); \
		Result = Expression->Execute(Context, nullptr); \
		REQUIRE_NO_ERROR(); \
		CHECK_VALUE(RESULT);
// end of TEST_EXPR_DEFAULT_CHECK_VALUE

#define TEST_EXPR_INVOKE_CHECK_VALUE(OBJ_VALUE, METHOD_NAME, RESULT, ...) \
		Expression = MakeShared<FInvokeExpression>( \
			EXPR_MEMBER(OBJ_VALUE, METHOD_NAME, false), \
			TMap<FString, TSharedPtr<FFormulaExpression>> { \
				PrepareArguments(TArray<TSharedPtr<FFormulaExpression>> { __VA_ARGS__ }) \
			} \
		); \
		INFO("Testing `" + Expression->ToString() + "` expression"); \
		Result = Expression->Execute(Context, nullptr); \
		REQUIRE_NO_ERROR(); \
		CHECK_VALUE(RESULT);
// end of TEST_EXPR_INVOKE_CHECK_VALUE

#define TEST_EXPR_LIST_INIT(COLLECTION_TYPE, COLL_PROPERTY,  ...) \
		Expression = MakeShared<FListInitExpression>( \
			MakeShared<FNewExpression>(COLLECTION_TYPE, TMap<FString, TSharedPtr<FFormulaExpression>>()), \
			TArray<TSharedPtr<FFormulaElementInitBinding>> { \
				__VA_ARGS__ \
			} \
		); \
		INFO("Testing `" + Expression->ToString() + "` expression"); \
		Result = Expression->Execute(Context, COLL_PROPERTY); \
		REQUIRE_NO_ERROR(); \
// end of TEST_EXPR_LIST_INIT

#define TEST_EXPR_NEW_ARRAY_INIT(COLLECTION_TYPE, COLL_PROPERTY,  ...) \
		Expression = MakeShared<FNewArrayInitExpression>( \
			COLLECTION_TYPE, \
			TArray<TSharedPtr<FFormulaExpression>> { \
				__VA_ARGS__ \
			} \
		); \
		INFO("Testing `" + Expression->ToString() + "` expression"); \
		Result = Expression->Execute(Context, COLL_PROPERTY); \
		REQUIRE_NO_ERROR(); \
// end of TEST_EXPR_LIST_INIT

#define TEST_EXPR_TYPE_IS(VALUE) \
		Expression = MakeShared<FTypeIsExpression>( \
			EXPR_CONST(VALUE), \
			MakeShared<FFormulaTypeReference>(FFormulaValue(VALUE).GetCPPType()) \
		); \
		INFO("Testing `" + Expression->ToString() + "` expression"); \
		Result = Expression->Execute(Context, nullptr); \
		REQUIRE_NO_ERROR(); \
		CHECK_VALUE(true);
// end of TEST_EXPR_TYPE_IS

#define TEST_EXPR_TYPE_IS_CLASS(VALUE, CLASS_NAME) \
		Expression = MakeShared<FTypeIsExpression>( \
			VALUE, \
			MakeShared<FFormulaTypeReference>(CLASS_NAME) \
		); \
		INFO("Testing `" + Expression->ToString() + "` expression"); \
		Result = Expression->Execute(Context, nullptr); \
		REQUIRE_NO_ERROR(); \
		CHECK_VALUE(true);
// end of TEST_EXPR_TYPE_IS_CLASS

#define TEST_EXPR_TYPE_IS_NOT(VALUE, TYPE_NAME) \
		Expression = MakeShared<FTypeIsExpression>( \
			VALUE, \
			MakeShared<FFormulaTypeReference>(TYPE_NAME) \
		); \
		INFO("Testing `" + Expression->ToString() + "` expression"); \
		Result = Expression->Execute(Context, nullptr); \
		REQUIRE_NO_ERROR(); \
		CHECK_VALUE(false);
// end of TEST_EXPR_TYPE_IS_NOT

#define TEST_EXPR_TYPE_OF(TYPE_NAME, EXPECTED_OBJ) \
		Expression = MakeShared<FTypeOfExpression>( \
			MakeShared<FFormulaTypeReference>(TYPE_NAME) \
		); \
		INFO("Testing `" + Expression->ToString() + "` expression"); \
		Result = Expression->Execute(Context, nullptr); \
		REQUIRE_NO_ERROR(); \
		CHECK_VALUE(static_cast<UObject*>(EXPECTED_OBJ));
// end of TEST_EXPR_TYPE_OF

#define TEST_EXPR_UNARY(V1, OP_T, OP) \
		Expression = MakeShared<FUnaryExpression>( \
			EXPR_CONST(V1), \
			OP_T \
		); \
		INFO("Testing `" + Expression->ToString() + "` expression"); \
		Result = Expression->Execute(Context, nullptr); \
		if constexpr (requires { OP  V1 ; }) \
		{ \
			REQUIRE_NO_ERROR(); \
			CHECK_VALUE(OP V1); \
		} \
		else \
		{ \
			REQUIRE_ERROR(EFormulaExecutionErrorCode::Op_UnaryNotSupported); \
		}
// end of TEST_EXPR_UNARY

#define TEST_EXPR_OBJ_MEMBER_CHECK_VALUE(OBJ_VALUE, PATH, RESULT, NULL_PROP) \
		Expression = EXPR_MEMBER(OBJ_VALUE, PATH, NULL_PROP); \
		INFO("Testing `" + Expression->ToString() + "` expression"); \
		Result = Expression->Execute(Context, nullptr); \
		REQUIRE_NO_ERROR(); \
		CHECK_VALUE(RESULT);
// end of TEST_EXPR_MEMBER_CHECK_VALUE

#define TEST_EXPR_GLOBAL_MEMBER_CHECK_VALUE(MEMBER_NAME, RESULT, NULL_PROP) \
		Expression =  MakeShared<FMemberExpression>(nullptr, FString(TEXT(MEMBER_NAME)), EmptyTypeArguments, NULL_PROP); \
		INFO("Testing `" + Expression->ToString() + "` expression"); \
		Result = Expression->Execute(Context, nullptr); \
		REQUIRE_NO_ERROR(); \
		CHECK_VALUE(RESULT);
// end of TEST_EXPR_MEMBER_CHECK_VALUE

inline TSharedRef<FJsonObject> FromJson(const FString& Text)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Text);
	FJsonSerializer::Deserialize(Reader, JsonObject);
	return JsonObject.ToSharedRef();
}

inline TMap<FString, TSharedPtr<FFormulaExpression>> PrepareArguments(TArray<TSharedPtr<FFormulaExpression>> Arguments)
{
	TMap<FString, TSharedPtr<FFormulaExpression>> ArgumentsByIndex;
	int32 Index = 0;
	for (auto Argument : Arguments)
	{
		ArgumentsByIndex.Add(FString::FromInt(Index), Argument);
		Index++;
	}
	return ArgumentsByIndex;
}

template <typename OutT>
OutT GetValue(const FFormulaExecutionResult Result)
{
	if (Result.HasError()) return  {}; 
	return Result.GetValue()->VisitValue([]<typename InT>(FProperty&, const InT& InValue) -> OutT { if constexpr (std::is_same_v<std::decay_t<InT>, OutT>) { return InValue; } else { return  {}; }});
}
template <typename ValueT>
bool IsValueSameType(const FFormulaExecutionResult Result)
{
	if (Result.HasError()) return  {}; 
	return Result.GetValue()->VisitValue([]<typename InT>(FProperty&, const InT&) -> bool { return std::is_same_v<std::decay_t<InT>, ValueT>; });
}

inline TArray<TSharedPtr<FFormulaTypeReference>> EmptyTypeArguments;
