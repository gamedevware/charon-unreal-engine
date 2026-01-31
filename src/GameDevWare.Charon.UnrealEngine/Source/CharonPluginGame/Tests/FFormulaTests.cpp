// Copyright GameDevWare, Denis Zykov 2025

#if WITH_TESTS
#pragma optimize("", off)

#include "limits"
#include "UFormulaTestObject.h"
#include "GameData/EGameDataFormat.h"
#include "GameData/FLocalizedText.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/UFormulaExpressionDefaultGlobal.h"
#include "GameData/Formulas/Expressions/FBinaryExpression.h"
#include "GameData/Formulas/Expressions/FConvertExpression.h"
#include "GameData/Formulas/Expressions/FConditionExpression.h"
#include "GameData/Formulas/Expressions/FConstantExpression.h"
#include "GameData/Formulas/Expressions/FDefaultExpression.h"
#include "GameData/Formulas/Expressions/FIndexExpression.h"
#include "GameData/Formulas/Expressions/FMemberExpression.h"
#include "GameData/Formulas/Expressions/FNewArrayBoundExpression.h"
#include "GameData/Formulas/Expressions/FNewExpression.h"
#include "GameData/Formulas/Expressions/FTypeIsExpression.h"
#include "GameData/Formulas/Expressions/FTypeOfExpression.h"
#include "GameData/Formulas/Expressions/FUnaryExpression.h"
#include "GameData/Formulas/Expressions/FInvokeExpression.h"
#include "GameData/Formulas/Expressions/FLambdaExpression.h"
#include "GameData/Formulas/Expressions/FListInitExpression.h"
#include "GameData/Formulas/Expressions/FMemberInitExpression.h"
#include "GameData/Formulas/Expressions/FFormulaElementInitBinding.h"
#include "GameData/Formulas/Expressions/FFormulaMemberAssignmentBinding.h"
#include "GameData/Formulas/Expressions/FFormulaMemberListBinding.h"
#include "GameData/Formulas/Expressions/FFormulaMemberMemberBinding.h"
#include "GameData/Formulas/Expressions/FNewArrayInitExpression.h"
#include "GameData/Formulas/Expressions/FMemberInitExpression.h"
#include "JsonUtils/JsonConversion.h"
#include "FFormulaTestStruct.h"
#include "Tests/TestHarnessAdapter.h"
#include "FFormulaTestMacros.h"
#include "GameData/FGameDataLoadOptions.h"

// TODO test Null prop
// TODO Test Struct access
// TODO Test enums

// ReSharper disable StringLiteralTypo

TEST_CASE_NAMED(FFormulaTests, "Charon::Formulas", "[Core]")
{
	FObjectPropertyBase* TestObjectProperty = CastFieldChecked<FObjectPropertyBase>(UFormulaTestObject::StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UFormulaTestObject, TestObjectProp)));
	FObjectPropertyBase* ActorProperty = CastFieldChecked<FObjectPropertyBase>(UFormulaTestObject::StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UFormulaTestObject, ActorProp)));
	FArrayProperty* IntArrayProperty = CastFieldChecked<FArrayProperty>(UFormulaTestObject::StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UFormulaTestObject, Int32Array)));
	FSetProperty* IntSetProperty = CastFieldChecked<FSetProperty>(UFormulaTestObject::StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UFormulaTestObject, Int32Set)));
	FMapProperty* IntMapProperty = CastFieldChecked<FMapProperty>(UFormulaTestObject::StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UFormulaTestObject, Int32Map)));
	FProperty* IntOutProperty = CastFieldChecked<FProperty>(UFormulaTestObject::StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(UFormulaTestObject, TestFunctionOutParam))->FindPropertyByName(TEXT("OutParameter")));

	TArray<UObject*> KnownTypes {
		AActor::StaticClass(),
		UFormulaTestObject::StaticClass(),
		UWorld::StaticClass(),
		UObject::StaticClass(),
		StaticEnum<EGameDataFormat>(),
		UFormulaExpressionDefaultGlobal::StaticClass(),
		FFormulaTestStruct::StaticStruct(),
		FLocalizedText::StaticStruct(),
	};
	auto Resolver = MakeShared<FFormulaTypeResolver>(nullptr, KnownTypes);
	auto TestActor = NewObject<AActor>();
	auto TestObject = NewObject<UFormulaTestObject>();
	auto TestObjectClassReference = MakeShared<FMemberExpression>(nullptr, UFormulaTestObject::StaticClass()->GetName(), EmptyTypeArguments, false);
	auto GlobalValue = MakeShared<FFormulaValue>(TestObjectProperty, &TestObject);
	TSharedPtr<FFormulaExpression> Expression;
	FFormulaExecutionResult Result = FFormulaExecutionResult(FFormulaExecutionError::UnsupportedExpression("#UNKNOWN"));
	FFormulaExecutionContext Context = FFormulaExecutionContext(
		/*bAutoNullPropagation*/ false,
		TMap<FString, const TSharedRef<FFormulaValue>> {
			{  TEXT("Arg1"), MakeShared<FFormulaValue>(1) },
			{  TEXT("Arg2"), MakeShared<FFormulaValue>(2) },
			{  TEXT("OutParameter"), MakeShared<FFormulaValue>(IntOutProperty, &TestObject->Int32Prop) },
		},
		GlobalValue,
		Resolver
	);

	SECTION("FConstantExpression")
	{
		// string based
		TEST_EXPR_CONST_CHECK_VALUE(FString(TEXT("1")), static_cast<uint8>(1));
		TEST_EXPR_CONST_CHECK_VALUE(FString(TEXT("2")), static_cast<uint16>(2));
		TEST_EXPR_CONST_CHECK_VALUE(FString(TEXT("3")), static_cast<uint32>(3));
		TEST_EXPR_CONST_CHECK_VALUE(FString(TEXT("4")), static_cast<uint64>(4));
		TEST_EXPR_CONST_CHECK_VALUE(FString(TEXT("-1")), static_cast<int8>(-1));
		TEST_EXPR_CONST_CHECK_VALUE(FString(TEXT("-2")), static_cast<int16>(-2));
		TEST_EXPR_CONST_CHECK_VALUE(FString(TEXT("-3")), static_cast<int32>(-3));
		TEST_EXPR_CONST_CHECK_VALUE(FString(TEXT("-4")), static_cast<int64>(-4));
		TEST_EXPR_CONST_CHECK_VALUE(FString(TEXT("-4")), static_cast<float>(-4));
		TEST_EXPR_CONST_CHECK_VALUE(FString(TEXT("-4")), static_cast<double>(-4));
		TEST_EXPR_CONST_CHECK_VALUE(FString(TEXT("true")), true);
		TEST_EXPR_CONST_CHECK_VALUE(FString(TEXT("false")), false);
		TEST_EXPR_CONST_CHECK_VALUE(FString(TEXT("a text")), FString(TEXT("a text")));
		TEST_EXPR_CONST_CHECK_VALUE(FString(TEXT("a text")), FText::FromString(TEXT("a text")));
		TEST_EXPR_CONST_CHECK_VALUE(FString(TEXT("a text")), FName(TEXT("a text")));

		// double based
		TEST_EXPR_CONST_CHECK_VALUE(static_cast<double>(1), static_cast<uint8>(1));
		TEST_EXPR_CONST_CHECK_VALUE(static_cast<double>(2), static_cast<uint16>(2));
		TEST_EXPR_CONST_CHECK_VALUE(static_cast<double>(3), static_cast<uint32>(3));
		TEST_EXPR_CONST_CHECK_VALUE(static_cast<double>(4), static_cast<uint64>(4));
		TEST_EXPR_CONST_CHECK_VALUE(static_cast<double>(-1), static_cast<int8>(-1));
		TEST_EXPR_CONST_CHECK_VALUE(static_cast<double>(-2), static_cast<int16>(-2));
		TEST_EXPR_CONST_CHECK_VALUE(static_cast<double>(-3), static_cast<int32>(-3));
		TEST_EXPR_CONST_CHECK_VALUE(static_cast<double>(-4), static_cast<int64>(-4));
		TEST_EXPR_CONST_CHECK_VALUE(static_cast<double>(-4), static_cast<float>(-4));
		TEST_EXPR_CONST_CHECK_VALUE(static_cast<double>(-4), static_cast<double>(-4));
		TEST_EXPR_CONST_CHECK_VALUE(static_cast<double>(1), true);
		TEST_EXPR_CONST_CHECK_VALUE(static_cast<double>(0), false);

		// bool based
		TEST_EXPR_CONST_CHECK_VALUE(true, true);
		TEST_EXPR_CONST_CHECK_VALUE(false, false);
		TEST_EXPR_CONST_CHECK_VALUE(true, FString(TEXT("True")));
		TEST_EXPR_CONST_CHECK_VALUE(false, FText::FromString(TEXT("False")));
		TEST_EXPR_CONST_CHECK_VALUE(false, FName(TEXT("False")));
	}
	
	SECTION("FBinaryExpression")
	{
		// supported operations
		TEST_EXPR_BINARY(static_cast<uint8>(1), static_cast<uint8>(1), EBinaryOperationType::And, &);
		TEST_EXPR_BINARY(static_cast<uint8>(15), static_cast<uint8>(10), EBinaryOperationType::Or, |);
		TEST_EXPR_BINARY(static_cast<uint8>(7), static_cast<uint8>(8), EBinaryOperationType::ExclusiveOr, ^);
		TEST_EXPR_BINARY(static_cast<uint8>(6), static_cast<uint8>(3), EBinaryOperationType::Divide, /);
		TEST_EXPR_BINARY(static_cast<uint8>(5), static_cast<uint8>(2), EBinaryOperationType::Modulo, %);
		TEST_EXPR_BINARY(static_cast<uint8>(1), static_cast<uint8>(1), EBinaryOperationType::Add, +);
		TEST_EXPR_BINARY(static_cast<uint8>(5), static_cast<uint8>(3), EBinaryOperationType::Subtract,-);
		TEST_EXPR_BINARY(static_cast<uint8>(66), static_cast<uint8>(1), EBinaryOperationType::LeftShift, <<);
		TEST_EXPR_BINARY(static_cast<uint8>(55), static_cast<uint8>(1), EBinaryOperationType::RightShift, >>);
		TEST_EXPR_BINARY(static_cast<uint8>(1), static_cast<uint8>(1), EBinaryOperationType::GreaterThan, >);
		TEST_EXPR_BINARY(static_cast<uint8>(1), static_cast<uint8>(1), EBinaryOperationType::GreaterThanOrEqual, >=);
		TEST_EXPR_BINARY(static_cast<uint8>(1), static_cast<uint8>(1), EBinaryOperationType::LessThan, <);
		TEST_EXPR_BINARY(static_cast<uint8>(1), static_cast<uint8>(1), EBinaryOperationType::LessThanOrEqual, <=);
		TEST_EXPR_BINARY(static_cast<uint8>(1), static_cast<uint8>(1), EBinaryOperationType::Equal, ==);
		TEST_EXPR_BINARY(static_cast<uint8>(1), static_cast<uint8>(1), EBinaryOperationType::NotEqual, !=);

		// number mix
		TEST_EXPR_BINARY(static_cast<uint8>(5), static_cast<int8>(1), EBinaryOperationType::Add, +);
		TEST_EXPR_BINARY(static_cast<uint16>(6), static_cast<uint32>(2), EBinaryOperationType::Add, +);
		TEST_EXPR_BINARY(static_cast<uint32>(7), static_cast<uint64>(3), EBinaryOperationType::Add, +);
		TEST_EXPR_BINARY(static_cast<uint64>(8), static_cast<uint64>(4), EBinaryOperationType::Add, +);
		TEST_EXPR_BINARY(static_cast<int8>(9), static_cast<int16>(5), EBinaryOperationType::Add, +);
		TEST_EXPR_BINARY(static_cast<int16>(1), static_cast<int32>(6), EBinaryOperationType::Add, +);
		TEST_EXPR_BINARY(static_cast<int32>(2), static_cast<int64>(7), EBinaryOperationType::Add, +);
		TEST_EXPR_BINARY(static_cast<int64>(3), static_cast<int64>(8), EBinaryOperationType::Add, +);
		TEST_EXPR_BINARY(static_cast<float>(4), static_cast<double>(9), EBinaryOperationType::Add, +);
		TEST_EXPR_BINARY(static_cast<double>(5), static_cast<float>(1), EBinaryOperationType::Add, +);
		TEST_EXPR_BINARY(static_cast<int32>(5), static_cast<float>(2), EBinaryOperationType::Add, +);
		TEST_EXPR_BINARY(static_cast<int32>(6), static_cast<double>(3), EBinaryOperationType::Add, +);
		TEST_EXPR_BINARY(static_cast<float>(7), static_cast<int32>(4), EBinaryOperationType::Add, +);
		TEST_EXPR_BINARY(static_cast<double>(8), static_cast<int32>(5), EBinaryOperationType::Add, +);
		
		// null lifted
		TEST_EXPR_BINARY_CHECK_VALUE(static_cast<int32>(123), nullptr, EBinaryOperationType::Add, nullptr);
		TEST_EXPR_BINARY_CHECK_VALUE(nullptr, static_cast<int32>(123), EBinaryOperationType::Add, nullptr);
		TEST_EXPR_BINARY_CHECK_VALUE(nullptr, nullptr, EBinaryOperationType::Add, nullptr);
		// comparison to null always false
		TEST_EXPR_BINARY_CHECK_VALUE(static_cast<int32>(123), nullptr, EBinaryOperationType::GreaterThan, false);
		TEST_EXPR_BINARY_CHECK_VALUE(static_cast<int32>(123), nullptr, EBinaryOperationType::GreaterThanOrEqual, false);
		TEST_EXPR_BINARY_CHECK_VALUE(static_cast<int32>(123), nullptr, EBinaryOperationType::LessThan, false);
		TEST_EXPR_BINARY_CHECK_VALUE(static_cast<int32>(123), nullptr, EBinaryOperationType::LessThanOrEqual, false);
		//  equals works as usual 
		TEST_EXPR_BINARY_CHECK_VALUE(static_cast<int32>(123), nullptr, EBinaryOperationType::Equal, false);
		TEST_EXPR_BINARY_CHECK_VALUE(static_cast<int32>(123), nullptr, EBinaryOperationType::NotEqual, true);
		
		// null lifted boolean OR and AND
		TEST_EXPR_BINARY_CHECK_VALUE(false, nullptr, EBinaryOperationType::And, false);
		TEST_EXPR_BINARY_CHECK_VALUE(nullptr, false, EBinaryOperationType::And, false);
		TEST_EXPR_BINARY_CHECK_VALUE(true, nullptr, EBinaryOperationType::Or, true);
		TEST_EXPR_BINARY_CHECK_VALUE(nullptr, true, EBinaryOperationType::Or, true);
		
		// logical junctions
		TEST_EXPR_BINARY(true, true, EBinaryOperationType::AndAlso, &&);
		TEST_EXPR_BINARY(true, true, EBinaryOperationType::OrElse, ||);
		TEST_EXPR_BINARY(false, true, EBinaryOperationType::AndAlso, &&);
		TEST_EXPR_BINARY(false, true, EBinaryOperationType::OrElse, ||);
		TEST_EXPR_BINARY(true, false, EBinaryOperationType::AndAlso, &&);
		TEST_EXPR_BINARY(true, false, EBinaryOperationType::OrElse, ||);
		
		// unsupported operations
		TEST_FAIL_EXPR_BINARY(static_cast<int8>(1), static_cast<int8>(1), EBinaryOperationType::AndAlso, &&);
		TEST_FAIL_EXPR_BINARY(static_cast<int8>(1), nullptr, EBinaryOperationType::AndAlso, &&);
		TEST_FAIL_EXPR_BINARY(static_cast<int8>(1), static_cast<int8>(1), EBinaryOperationType::OrElse, ||);
		
		// unsupported mixed type operations
		TEST_FAIL_EXPR_BINARY(true, static_cast<uint8>(1), EBinaryOperationType::Add, +);
		TEST_FAIL_EXPR_BINARY(static_cast<uint8>(1), true, EBinaryOperationType::Or,|);
		
		// coalesce 
		TEST_EXPR_BINARY_CHECK_VALUE(nullptr, 1, EBinaryOperationType::Coalesce, 1);
		TEST_EXPR_BINARY_CHECK_VALUE(2, 1, EBinaryOperationType::Coalesce, 2);
		TEST_EXPR_BINARY_CHECK_VALUE(nullptr, nullptr, EBinaryOperationType::Coalesce, nullptr);
		
		// power
		TEST_EXPR_BINARY_CHECK_VALUE(2, 2, EBinaryOperationType::Power, 4.0);
	}

	SECTION("FConditionExpression")
	{
		Expression = MakeShared<FConditionExpression>(
			EXPR_CONST(true),
			EXPR_CONST(1),
			EXPR_CONST(2)
		);
		Result = Expression->Execute(Context, nullptr);
		REQUIRE_NO_ERROR();
		CHECK_VALUE(1);

		Expression = MakeShared<FConditionExpression>(
			EXPR_CONST(false),
			EXPR_CONST(1),
			EXPR_CONST(2)
		);
		Result = Expression->Execute(Context, nullptr);
		REQUIRE_NO_ERROR();
		CHECK_VALUE(2);
	}

	SECTION("FConvertExpression")
	{
		// number conversion
		TEST_EXPR_CONVERT_CHECK_VALUE(static_cast<int32>(10), static_cast<uint8>(10), FFormulaNotation::EXPRESSION_TYPE_CONVERT);
		TEST_EXPR_CONVERT_CHECK_VALUE(static_cast<uint8>(110), static_cast<int32>(110), FFormulaNotation::EXPRESSION_TYPE_CONVERT);
		TEST_EXPR_CONVERT_CHECK_VALUE(static_cast<int32>(-120), static_cast<float>(-120), FFormulaNotation::EXPRESSION_TYPE_CONVERT);
		TEST_EXPR_CONVERT_CHECK_VALUE(static_cast<int32>(-120), static_cast<double>(-120), FFormulaNotation::EXPRESSION_TYPE_CONVERT);
		TEST_EXPR_CONVERT_CHECK_VALUE(static_cast<float>(-120), static_cast<int32>(-120), FFormulaNotation::EXPRESSION_TYPE_CONVERT);
		TEST_EXPR_CONVERT_CHECK_VALUE(static_cast<double>(-120), static_cast<int32>(-120), FFormulaNotation::EXPRESSION_TYPE_CONVERT);
		TEST_EXPR_CONVERT_CHECK_VALUE(static_cast<int32>(10), true, FFormulaNotation::EXPRESSION_TYPE_CONVERT);
		TEST_EXPR_CONVERT_CHECK_VALUE(static_cast<int32>(0), false, FFormulaNotation::EXPRESSION_TYPE_CONVERT);
		
		// FName, FText, FString conversion
		TEST_EXPR_CONVERT_CHECK_VALUE(FName(TEXT("F_NAME_1_ř")), FString(TEXT("F_NAME_1_ř")), FFormulaNotation::EXPRESSION_TYPE_CONVERT);
		TEST_EXPR_CONVERT_CHECK_VALUE(FName(TEXT("F_NAME_2_ř")), FName(TEXT("F_NAME_2_ř")), FFormulaNotation::EXPRESSION_TYPE_CONVERT);
		TEST_EXPR_CONVERT_CHECK_VALUE(FText::FromString(TEXT("F_TEXT_3_ř")), FString(TEXT("F_TEXT_3_ř")), FFormulaNotation::EXPRESSION_TYPE_CONVERT);
		TEST_EXPR_CONVERT_CHECK_VALUE(FText::FromString(TEXT("F_TEXT_4_ř")), FText::FromString(TEXT("F_TEXT_4_ř")), FFormulaNotation::EXPRESSION_TYPE_CONVERT);
		TEST_EXPR_CONVERT_CHECK_VALUE(FString(TEXT("F_STRING_5_ř")), FName(TEXT("F_STRING_5_ř")), FFormulaNotation::EXPRESSION_TYPE_CONVERT);
		TEST_EXPR_CONVERT_CHECK_VALUE(FString(TEXT("F_STRING_6_ř")), FString(TEXT("F_STRING_6_ř")), FFormulaNotation::EXPRESSION_TYPE_CONVERT);
		TEST_EXPR_CONVERT_CHECK_VALUE(FString(TEXT("F_STRING_7_ř")), FText::FromString(TEXT("F_STRING_7_ř")), FFormulaNotation::EXPRESSION_TYPE_CONVERT);

		// Type as - downcast
		TEST_EXPR_CONVERT_TYPED_CHECK_VALUE(EXPR_CONST_NULL(), "AActor", nullptr, FFormulaNotation::EXPRESSION_TYPE_TYPE_AS);
		TEST_EXPR_CONVERT_TYPED_CHECK_VALUE(EXPR_CONST_OBJECT(TestObject, TestObjectProperty), "UFormulaTestObject", Cast<UObject>(TestObject), FFormulaNotation::EXPRESSION_TYPE_TYPE_AS);
		TEST_EXPR_CONVERT_TYPED_CHECK_VALUE(EXPR_CONST_OBJECT(TestObject, TestObjectProperty), "UWorld", nullptr, FFormulaNotation::EXPRESSION_TYPE_TYPE_AS);

		// Type as upcast
		TEST_EXPR_CONVERT_TYPED_CHECK_VALUE(EXPR_CONST_OBJECT(TestActor, ActorProperty), "UObject", Cast<UObject>(TestActor), FFormulaNotation::EXPRESSION_TYPE_TYPE_AS);
	}

	SECTION("FDefaultExpression")
	{
		TEST_EXPR_DEFAULT_CHECK_VALUE(false);
		TEST_EXPR_DEFAULT_CHECK_VALUE(static_cast<int8>(0));
		TEST_EXPR_DEFAULT_CHECK_VALUE(static_cast<int16>(0));
		TEST_EXPR_DEFAULT_CHECK_VALUE(static_cast<int32>(0));
		TEST_EXPR_DEFAULT_CHECK_VALUE(static_cast<int64>(0));
		TEST_EXPR_DEFAULT_CHECK_VALUE(static_cast<uint8>(0));
		TEST_EXPR_DEFAULT_CHECK_VALUE(static_cast<uint16>(0));
		TEST_EXPR_DEFAULT_CHECK_VALUE(static_cast<uint32>(0));
		TEST_EXPR_DEFAULT_CHECK_VALUE(static_cast<uint64>(0));
		TEST_EXPR_DEFAULT_CHECK_VALUE(static_cast<float>(0));
		TEST_EXPR_DEFAULT_CHECK_VALUE(static_cast<double>(0));
		TEST_EXPR_DEFAULT_CHECK_VALUE(FString());
		TEST_EXPR_DEFAULT_CHECK_VALUE(FText());
		TEST_EXPR_DEFAULT_CHECK_VALUE(FName());
		TEST_EXPR_DEFAULT_CHECK_VALUE(nullptr);
	}

	SECTION("FIndexExpression")
	{
		TestObject->Int32Array = TArray { 123, 456, 789, 1011, -1213, 1415, 1617 };
		TestObject->Int32Set = TSet { 123, 456, 789, 1011, -1213, 1415, 1617 };
		TestObject->Int32Map = TMap<FString, int32> { { FString(TEXT("key")), -1213 } };
		auto TestObjectExpr = EXPR_CONST_OBJECT(TestObject, TestObjectProperty);
		
		TEST_EXPR_INDEX_CHECK_VALUE(TestObjectExpr, "Int32Array", 4, -1213, false);
		TEST_EXPR_INDEX_CHECK_VALUE(TestObjectExpr, "Int32Set", 4, -1213, false);
		TEST_EXPR_INDEX_CHECK_VALUE(TestObjectExpr, "Int32Map", FString(TEXT("key")), -1213, false);
	}

	SECTION("FInvokeExpression")
	{
		TestObject->Int32Prop = 54321;
		auto StaticInvokeResult = UFormulaTestObject::StaticClass()->GetDefaultObject<UFormulaTestObject>()->Int32Prop;
		
		// TODO: invocable obj call (a + b)()
		
		// member call
		TEST_EXPR_INVOKE_CHECK_VALUE(EXPR_CONST_OBJECT(TestObject, TestObjectProperty), "TestFunctionUint8Param", 54321 + 123, EXPR_CONST(123));
		TEST_EXPR_INVOKE_CHECK_VALUE(EXPR_CONST_OBJECT(TestObject, TestObjectProperty), "TestFunctionNoParam", 54321);
		// null propagation member call
		TEST_EXPR_INVOKE_CHECK_VALUE(EXPR_MEMBER(EXPR_CONST_OBJECT(TestObject, TestObjectProperty), "TestObjectProp", true), "TestFunctionNoParam", nullptr);
		// static function call
		TEST_EXPR_INVOKE_CHECK_VALUE(TestObjectClassReference, "StaticTestFunctionNoParam", StaticInvokeResult);
		// global function call
		TEST_EXPR_INVOKE_CHECK_VALUE(nullptr, "TestFunctionNoParam", 54321);
		
		// out parameters
		TestObject->Int32Prop = -123123;
		TEST_EXPR_INVOKE_CHECK_VALUE(nullptr, "TestFunctionOutParam", -123123, EXPR_ARG("OutParameter"));
		auto OutParameterPtr = Context.Arguments.Find(TEXT("OutParameter"));
		REQUIRE(OutParameterPtr);
		int32 OutParameterValue;
		CHECK(OutParameterPtr->Get().TryGetInt32(OutParameterValue));
		CHECK_EQUALS("Out Parameter", OutParameterValue, -123123);
	}
	
	SECTION("FListInitExpression")
	{
		TEST_EXPR_LIST_INIT(MAKE_ARRAY_TYPE_REF("int32"), IntArrayProperty, EXPR_ELEM_INIT(EXPR_CONST(-1)), EXPR_ELEM_INIT(EXPR_CONST(-6)), EXPR_ELEM_INIT(EXPR_CONST(-10764)), EXPR_ELEM_INIT(EXPR_CONST(214132342)));
		
		TSharedPtr<FFormulaValue> ListResult = Result.GetValue();
		void* ResultArrayPtr;
		CHECK(ListResult->TryGetContainerAddress(ResultArrayPtr));
		CHECK(ResultArrayPtr);
		TArray<int32>* ResultArray = static_cast<TArray<int32>*>(ResultArrayPtr);
		REQUIRE(ResultArray->Num() == 4);
		CHECK_EQUALS("Item1", -1, (*ResultArray)[0]);
		CHECK_EQUALS("Item2", -6, (*ResultArray)[1]);
		CHECK_EQUALS("Item3", -10764, (*ResultArray)[2]);
		CHECK_EQUALS("Item4", 214132342, (*ResultArray)[3]);
		ResultArray = nullptr;
		ResultArrayPtr = nullptr;
		
		TEST_EXPR_LIST_INIT(MAKE_SET_TYPE_REF("int32"), IntSetProperty, EXPR_ELEM_INIT(EXPR_CONST(-1)), EXPR_ELEM_INIT(EXPR_CONST(-6)), EXPR_ELEM_INIT(EXPR_CONST(-10764)), EXPR_ELEM_INIT(EXPR_CONST(214132342)));
		
		ListResult = Result.GetValue();
		void* ResultSetPtr;
		CHECK(ListResult->TryGetContainerAddress(ResultSetPtr));
		CHECK(ResultSetPtr);
		TSet<int32>* ResultSet = static_cast<TSet<int32>*>(ResultSetPtr);
		REQUIRE(ResultSet->Num() == 4);
		CHECK(ResultSet->Contains(-1));
		CHECK(ResultSet->Contains(-6));
		CHECK(ResultSet->Contains(-10764));
		CHECK(ResultSet->Contains(214132342));
		ResultSet = nullptr;
		ResultSetPtr = nullptr;
		
		FString Key1 = TEXT("Item1");
		FString Key2 = TEXT("AKey");
		FString Key3 = TEXT("Another");
		FString Key4 = TEXT("Extra");
		
		TEST_EXPR_LIST_INIT(MAKE_MAP_TYPE_REF("string", "int32"), IntMapProperty, 
			EXPR_ELEM_INIT(EXPR_CONST(Key1), EXPR_CONST(-1)), 
			EXPR_ELEM_INIT(EXPR_CONST(Key2), EXPR_CONST(-6)), 
			EXPR_ELEM_INIT(EXPR_CONST(Key3), EXPR_CONST(-10764)), 
			EXPR_ELEM_INIT(EXPR_CONST(Key4), EXPR_CONST(214132342)));
		
		ListResult = Result.GetValue();
		void* ResultMapPtr;
		CHECK(ListResult->TryGetContainerAddress(ResultMapPtr));
		CHECK(ResultMapPtr);
		TMap<FString, int32>* ResultMap = static_cast<TMap<FString, int32>*>(ResultMapPtr);
		REQUIRE(ResultMap->Num() == 4);
		CHECK(ResultMap->Find(Key1));
		CHECK(ResultMap->Find(Key2));
		CHECK(ResultMap->Find(Key3));
		CHECK(ResultMap->Find(Key4));
		CHECK_EQUALS("Item1", -1, *ResultMap->Find(Key1));
		CHECK_EQUALS("Item2", -6, *ResultMap->Find(Key2));
		CHECK_EQUALS("Item3", -10764, *ResultMap->Find(Key3));
		CHECK_EQUALS("Item4", 214132342, *ResultMap->Find(Key4));
		ResultMap = nullptr;
		ResultMapPtr = nullptr;
		
		// Forces the compiler to acknowledge Result
		(void)ListResult;
	}

	SECTION("FNewArrayInitExpression")
	{
		TEST_EXPR_NEW_ARRAY_INIT(MAKE_ARRAY_TYPE_REF("int32"), IntArrayProperty, EXPR_CONST(-1), EXPR_CONST(-6), EXPR_CONST(-10764), EXPR_CONST(214132342));
		
		TSharedPtr<FFormulaValue> ArrayResult = Result.GetValue();
		void* ResultArrayPtr;
		CHECK(ArrayResult->TryGetContainerAddress(ResultArrayPtr));
		CHECK(ResultArrayPtr);
		TArray<int32>* ResultArray = static_cast<TArray<int32>*>(ResultArrayPtr);
		REQUIRE(ResultArray->Num() == 4);
		CHECK_EQUALS("Item1", -1, (*ResultArray)[0]);
		CHECK_EQUALS("Item2", -6, (*ResultArray)[1]);
		CHECK_EQUALS("Item3", -10764, (*ResultArray)[2]);
		CHECK_EQUALS("Item4", 214132342, (*ResultArray)[3]);
		
		// Forces the compiler to acknowledge Result
		(void)ArrayResult;
	}

	SECTION("FMemberInitExpression")
	{
		Expression = MakeShared<FMemberInitExpression>(
			MakeShared<FNewExpression>(
				MakeShared<FFormulaTypeReference>("FormulaTestObject"),
				TMap<FString, TSharedPtr<FFormulaExpression>>()
			),
			TArray<TSharedPtr<FFormulaMemberBinding>> {
				MakeShared<FFormulaMemberAssignmentBinding>("StringProp", EXPR_CONST(FString(TEXT("NEW_STRING_VALUE")))),
				MakeShared<FFormulaMemberAssignmentBinding>("Int32Prop", EXPR_CONST(-1000)),
				MakeShared<FFormulaMemberListBinding>("Int32Array", TArray<TSharedPtr<FFormulaExpression>> { EXPR_CONST(123) }),
				MakeShared<FFormulaMemberListBinding>("Int32Array", TArray<TSharedPtr<FFormulaExpression>> { EXPR_CONST(321) }),
				MakeShared<FFormulaMemberListBinding>("Int32Set", TArray<TSharedPtr<FFormulaExpression>> { EXPR_CONST(321) }),
				MakeShared<FFormulaMemberListBinding>("Int32Map", TArray<TSharedPtr<FFormulaExpression>> { EXPR_CONST(FString(TEXT("AMapKey"))), EXPR_CONST(321) }),
				MakeShared<FFormulaMemberAssignmentBinding>("TestObjectProp", EXPR_CONST_OBJECT(TestObject, TestObjectProperty)),
				MakeShared<FFormulaMemberMemberBinding>("TestObjectProp", TArray<TSharedPtr<FFormulaMemberBinding>> 
				{ 
					MakeShared<FFormulaMemberAssignmentBinding>("Int32Prop", EXPR_CONST(-2000)),
				})
			}
		);
		INFO("Testing `" + Expression->ToString() + "` expression");
		Result = Expression->Execute(Context, TestObjectProperty);
		REQUIRE_NO_ERROR();
		UObject* CreatedObject;
		CHECK(Result.GetValue()->TryCopyCompleteValue(UDotNetObject::GetLiteralProperty(), &CreatedObject));
		UFormulaTestObject* CreateInitializedObject = Cast<UFormulaTestObject>(CreatedObject);
		REQUIRE(CreateInitializedObject);
		
		CHECK_EQUALS("StringProp", CreateInitializedObject->StringProp, FString(TEXT("NEW_STRING_VALUE")));
		CHECK_EQUALS("Int32Prop", CreateInitializedObject->Int32Prop, -1000);
		CHECK_EQUALS("Int32Array.Num()", CreateInitializedObject->Int32Array.Num(), 2);
		if (CreateInitializedObject->Int32Array.Num() == 2)
		{
			CHECK_EQUALS("Int32Array[0]", CreateInitializedObject->Int32Array[0], 123);
			CHECK_EQUALS("Int32Array[1]", CreateInitializedObject->Int32Array[1], 321);
		}
		CHECK_EQUALS("Int32Set.Num()", CreateInitializedObject->Int32Set.Num(), 1);
		if (CreateInitializedObject->Int32Set.Num() == 1)
		{
			CHECK_MESSAGE("Int32Set[0]", CreateInitializedObject->Int32Set.Contains(321));
		}
		CHECK_EQUALS("Int32Map.Num()", CreateInitializedObject->Int32Map.Num(), 1);
		if (CreateInitializedObject->Int32Map.Num() == 1)
		{
			CHECK_EQUALS("Int32Map[AMapKey]", *CreateInitializedObject->Int32Map.Find(FString(TEXT("AMapKey"))), 321);
		}
		CHECK_NOT_EQUALS("TestObjectProp", CreateInitializedObject->TestObjectProp, static_cast<UFormulaTestObject*>(nullptr));
		if (CreateInitializedObject->TestObjectProp)
		{
			CHECK_EQUALS("TestObjectProp.Int32Prop", CreateInitializedObject->TestObjectProp->Int32Prop, -2000);
		}
	}
	
	SECTION("FLambdaExpression")
	{
		
	}

	SECTION("FMemberExpression")
	{
		TestObject->Int32Prop = 123456;
		UFormulaTestObject* NullTestObject = nullptr;

		// member access
		TEST_EXPR_OBJ_MEMBER_CHECK_VALUE(EXPR_CONST_OBJECT(TestObject, TestObjectProperty), "Int32Prop", 123456, false);

		// null propagation member access
		TEST_EXPR_OBJ_MEMBER_CHECK_VALUE(EXPR_CONST_OBJECT(NullTestObject, TestObjectProperty), "Int32Prop", nullptr, true);

		// static member access
		auto TestObjectDefaultInt32Value = UFormulaTestObject::StaticClass()->GetDefaultObject<UFormulaTestObject>()->Int32Prop;
		TEST_EXPR_OBJ_MEMBER_CHECK_VALUE(TestObjectClassReference, "Int32Prop", TestObjectDefaultInt32Value, false);
		
		// argument access
		TEST_EXPR_GLOBAL_MEMBER_CHECK_VALUE("Arg1", 1, false);
		TEST_EXPR_GLOBAL_MEMBER_CHECK_VALUE("Arg2", 2, false);

		// global member access
		TEST_EXPR_GLOBAL_MEMBER_CHECK_VALUE("Int32Prop", TestObject->Int32Prop, false);

		// literals
		TEST_EXPR_GLOBAL_MEMBER_CHECK_VALUE("null", nullptr, false);
		TEST_EXPR_GLOBAL_MEMBER_CHECK_VALUE("true", true, false);
		TEST_EXPR_GLOBAL_MEMBER_CHECK_VALUE("false", false, false);
	}

	SECTION("FNewArrayBoundExpression")
	{
		Expression = MakeShared<FNewArrayBoundExpression>(
			MAKE_ARRAY_TYPE_REF("int32"),
			TMap<FString, TSharedPtr<FFormulaExpression>> {
				{ FString(TEXT("0")), EXPR_CONST(10) }
			}
		);
		INFO("Testing `" + Expression->ToString() + "` expression");
		Result = Expression->Execute(Context, nullptr);
		REQUIRE_NO_ERROR();
		TArray<int32> Int32Array;
		CHECK(Result.GetValue()->TryCopyCompleteValue(UDotNetInt32::GetArrayProperty(), &Int32Array));
		CHECK_EQUALS("Array size", Int32Array.Num(), 10);
	}

	SECTION("FNewExpression")
	{
		Expression = MakeShared<FNewExpression>(
			MakeShared<FFormulaTypeReference>("Actor"),
			TMap<FString, TSharedPtr<FFormulaExpression>>()
		);
		INFO("Testing `" + Expression->ToString() + "` expression");
		Result = Expression->Execute(Context, nullptr);
		REQUIRE_NO_ERROR();
		UObject* CreatedObject;
		CHECK(Result.GetValue()->TryCopyCompleteValue(UDotNetObject::GetLiteralProperty(), &CreatedObject));
		CHECK_NOT_EQUALS("Object Pointer", CreatedObject, static_cast<UObject*>(nullptr));
		
		// Forces the compiler to acknowledge Result
		(void)Result;
	}

	SECTION("FTypeIsExpression")
	{
		TEST_EXPR_TYPE_IS(false);
		TEST_EXPR_TYPE_IS(static_cast<int8>(0));
		TEST_EXPR_TYPE_IS(static_cast<int16>(0));
		TEST_EXPR_TYPE_IS(static_cast<int32>(0));
		TEST_EXPR_TYPE_IS(static_cast<int64>(0));
		TEST_EXPR_TYPE_IS(static_cast<uint8>(0));
		TEST_EXPR_TYPE_IS(static_cast<uint16>(0));
		TEST_EXPR_TYPE_IS(static_cast<uint32>(0));
		TEST_EXPR_TYPE_IS(static_cast<uint64>(0));
		TEST_EXPR_TYPE_IS(static_cast<float>(0));
		TEST_EXPR_TYPE_IS(static_cast<double>(0));
		TEST_EXPR_TYPE_IS(FString());
		TEST_EXPR_TYPE_IS(FText());
		TEST_EXPR_TYPE_IS(FName());
		TEST_EXPR_TYPE_IS(nullptr);
		
		TEST_EXPR_TYPE_IS_CLASS(EXPR_CONST_OBJECT(TestObject, TestObjectProperty), TestObject->StaticClass()->GetName());
		TEST_EXPR_TYPE_IS_CLASS(EXPR_CONST_OBJECT(TestObject, TestObjectProperty), UObject::StaticClass()->GetName());

		TEST_EXPR_TYPE_IS_NOT(EXPR_CONST_OBJECT(TestObject, TestObjectProperty), "int32");
		TEST_EXPR_TYPE_IS_NOT(EXPR_CONST(FString()), "Object");
		TEST_EXPR_TYPE_IS_NOT(EXPR_CONST(static_cast<int8>(0)), "float");
	}

	SECTION("FTypeOfExpression")
	{
		TEST_EXPR_TYPE_OF(TEXT("UObject"), UObject::StaticClass());
		TEST_EXPR_TYPE_OF(TEXT("AActor"), AActor::StaticClass());
		TEST_EXPR_TYPE_OF(TEXT("int32"), UDotNetInt32::StaticClass());
		TEST_EXPR_TYPE_OF(TEXT("FString"), UDotNetString::StaticClass());
		TEST_EXPR_TYPE_OF(TEXT("String"), UDotNetString::StaticClass());
		TEST_EXPR_TYPE_OF(TEXT("EGameDataFormat"), StaticEnum<EGameDataFormat>());
		TEST_EXPR_TYPE_OF(TEXT("GameDataFormat"), StaticEnum<EGameDataFormat>());
		TEST_EXPR_TYPE_OF(TEXT("FLocalizedText"), FLocalizedText::StaticStruct());
		TEST_EXPR_TYPE_OF(TEXT("LocalizedText"), FLocalizedText::StaticStruct());
	}

	SECTION("FUnaryExpression")
	{
		TEST_EXPR_UNARY(123, EUnaryOperationType::UnaryPlus, +);
		TEST_EXPR_UNARY(1234, EUnaryOperationType::Negate, -);
		TEST_EXPR_UNARY(12345, EUnaryOperationType::Complement, ~);
		TEST_EXPR_UNARY(true, EUnaryOperationType::Not, !);
		TEST_EXPR_UNARY(false, EUnaryOperationType::Not, !);

		// no-ops
		TEST_EXPR_UNARY(123456, EUnaryOperationType::Group, +);
		TEST_EXPR_UNARY(1234567, EUnaryOperationType::CheckedScope, +);
		TEST_EXPR_UNARY(12345678, EUnaryOperationType::UncheckedScope, +);
	}
}
#endif

