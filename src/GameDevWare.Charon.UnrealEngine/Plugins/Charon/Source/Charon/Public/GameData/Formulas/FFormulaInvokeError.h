#pragma once

#include "FFormulaInvokeResult.h"
#include "Misc/TVariant.h"

enum class EFormulaInvokeErrorCode
{
	InvalidCastError,
	CantConvertToType,
	CanConvertNullToType,
	InvalidConditionResultType,
	UnableToResolveType,
	CantFindIndexer,
	InvalidDictionaryKeyType,
	IndexOutOfRange,
	DictionaryKeyNotFound,
	NullReference,
	ExpressionIsInvalid,
	UnableToResolveGlobalName,
	InvalidArrayType,
	UnsupportedArrayType,
	InvalidArraySizeValue,
	UnsupportedObjectType,
	CantFindMember,
	CantFindStaticMember,
	UnableToBindMethodToParameters,
	MissingUnaryOperation,
	MissingBinaryOperation, UnableToCreateTypeInstance, CanCreateInstanceOfAbstractClass
};

class FFormulaInvokeError
{
public:
	const FString Message;
	const EFormulaInvokeErrorCode Code;

	FFormulaInvokeError(const FString& Message, const EFormulaInvokeErrorCode Code)
		: Message(Message), Code(Code)
	{
		
	}

	static FFormulaInvokeError InvalidCastError(FString FromTypeName, FString ToTypeName)
	{
		return FFormulaInvokeError(
			FString::Format(TEXT("Object of '{0}' type is not convertable to '{1}' type."), { FromTypeName, ToTypeName }),
			EFormulaInvokeErrorCode::InvalidCastError
		);
	}

	static FFormulaInvokeError CanConvertNullToType(const FString& ToTypeName)
	{
		return FFormulaInvokeError(
			FString::Format(TEXT("Can convert <null>  to '{0}' type."), { ToTypeName }),
			EFormulaInvokeErrorCode::CanConvertNullToType
		);
	}
	static FFormulaInvokeError CantConvertToType(FString FromTypeName, FString ToTypeName)
	{
		return FFormulaInvokeError(
			FString::Format(TEXT("Can convert from '{0}' type to '{1}' type."), { FromTypeName, ToTypeName }),
			EFormulaInvokeErrorCode::CantConvertToType
		);
	}
	static FFormulaInvokeError InvalidConditionResultType(FString TypeName)
	{
		return FFormulaInvokeError(
			FString::Format(TEXT("Invalid type '{0}' for result of conditional expression."), { TypeName }),
			EFormulaInvokeErrorCode::InvalidConditionResultType
		);
	}
	static FFormulaInvokeError UnableToResolveType(FString TypeName)
	{
		return FFormulaInvokeError(
			FString::Format(TEXT("Can't find type with name '{0}'. Add this type to list of known types before using it."), { TypeName }),
			EFormulaInvokeErrorCode::UnableToResolveType
		);
	}
	static FFormulaInvokeError CantFindIndexer(const FString& ListType, const FString& ArgumentTypes)
	{
		return FFormulaInvokeError(
			FString::Format(TEXT("Can't find indexer on type '{0}' accepting {1} arguments."), { ListType, ArgumentTypes }),
			EFormulaInvokeErrorCode::CantFindIndexer
		);
	}
	static FFormulaInvokeError InvalidDictionaryKeyType(const FString& KeyValue, const FString& KeyType, const FString& ExpectedKeyType)
	{
		return FFormulaInvokeError(
			FString::Format(TEXT("Invalid map key '{0}'[{1}] provided while '{2}' is expected."), { KeyValue, KeyType, ExpectedKeyType }),
			EFormulaInvokeErrorCode::InvalidDictionaryKeyType
		);
	}
	static FFormulaInvokeError IndexOutOfRange(int32 Index, int32 CollectionCount)
	{
		return FFormulaInvokeError(
			FString::Format(TEXT("Index {0} is outside of bounds {1} of collection."), { Index, CollectionCount }),
			EFormulaInvokeErrorCode::IndexOutOfRange
		);
	}
	static FFormulaInvokeError DictionaryKeyNotFound(const FString& KeyValue, const FString& KeyType)
	{
		return FFormulaInvokeError(
			FString::Format(TEXT("Key '{0}'[{1}] is not found in map."), { KeyValue, KeyType }),
			EFormulaInvokeErrorCode::DictionaryKeyNotFound
		);
	}
	static FFormulaInvokeError NullReference()
	{
		return FFormulaInvokeError(
			TEXT("Object reference not set to an instance of an object."),
			EFormulaInvokeErrorCode::NullReference
		);
	}
	static FFormulaInvokeError CantFindMember(const FString& TypeName, const FString& MemberName, const FString& AllMemberName)
	{
		return FFormulaInvokeError(
			FString::Format(TEXT("Can find property/function with name '{1}' on '{0}' type. Available properties/functions: {2}."), { TypeName, MemberName, AllMemberName }),
			EFormulaInvokeErrorCode::CantFindMember
		);
	}
	static FFormulaInvokeError CantFindStaticMember(const FString& TypeName, const FString& MemberName, const FString& AllMemberName)
	{
		return FFormulaInvokeError(
			FString::Format(TEXT("Can find static property/function with name '{1}' on '{0}' type. Available static properties/functions: {2}."), { TypeName, MemberName, AllMemberName }),
			EFormulaInvokeErrorCode::CantFindStaticMember
		);
	}
	static FFormulaInvokeError ExpressionIsInvalid()
	{
		return FFormulaInvokeError(
			TEXT("FFormulaExpression object is in invalid state."),
			EFormulaInvokeErrorCode::ExpressionIsInvalid
		);
	}
	static FFormulaInvokeError UnableToResolveGlobalName(const FString& MemberName, const FString& AllGlobalMembers)
	{
		return FFormulaInvokeError(
			FString::Format(TEXT("Can find global member or argument with '{0}' name. Available members and arguments: {1}."), { MemberName, AllGlobalMembers }),
			EFormulaInvokeErrorCode::UnableToResolveGlobalName
		);
	}
	static FFormulaInvokeError InvalidArrayType(const FString& TypeName)
	{
		return FFormulaInvokeError(
			FString::Format(TEXT("An array type '{0}' should be 'Array' and have only one type argument which it's element type."), { TypeName }),
			EFormulaInvokeErrorCode::InvalidArrayType
		);
	}
	static FFormulaInvokeError UnsupportedArrayType(const FString& TypeName)
	{
		return FFormulaInvokeError(
			FString::Format(TEXT("An array type '{0}' is not supported. Only UObject derived types and basic structure types are supported. "), { TypeName }),
			EFormulaInvokeErrorCode::UnsupportedArrayType
		);
	}
	static FFormulaInvokeError InvalidArraySizeValue(const FString& SizeValueTypeName)
	{
		return FFormulaInvokeError(
			FString::Format(TEXT("Invalid type '{0}' for array's size value. Integer value is expected. "), { SizeValueTypeName }),
			EFormulaInvokeErrorCode::InvalidArraySizeValue
		);
	}
	static FFormulaInvokeError UnsupportedObjectType(const FString& TypeName)
	{
		return FFormulaInvokeError(
			FString::Format(TEXT("Unsupported type '{0}' construction. Only UCLASS() reflection-visible types and primitive ones are supported."), { TypeName }),
			EFormulaInvokeErrorCode::UnsupportedObjectType
		);
	}
	static FFormulaInvokeError UnableToBindMethodToParameters(const FString& TypeName, const FString& FunctionName, const FString& ArgumentTypes, int32 ArgumentCount)
	{
		if (ArgumentCount != 0)
		{
			return FFormulaInvokeError(
				FString::Format(TEXT("Can't find method with name '{0}' on type '{1}' accepting {2} arguments of types."), { TypeName, FunctionName, ArgumentTypes }),
				EFormulaInvokeErrorCode::UnableToBindMethodToParameters
			);
		}
		else
		{
			return FFormulaInvokeError(
				FString::Format(TEXT("Can't find method with name '{0}' on type '{1}' accepting no arguments."), { TypeName, FunctionName }),
				EFormulaInvokeErrorCode::UnableToBindMethodToParameters
			);
		}
	}
	static FFormulaInvokeError MissingUnaryOperation(const FString& TypeName, const FString& UnaryOperation)
	{
		return FFormulaInvokeError(
			FString::Format(TEXT("Missing unary operation '{0}' on type '{1}'."), { UnaryOperation, TypeName }),
			EFormulaInvokeErrorCode::MissingUnaryOperation
		);
	}
	static FFormulaInvokeError MissingBinaryOperation(const FString& LeftTypeName, const FString& RightTypeName, const FString& BinaryOperation)
	{
		return FFormulaInvokeError(
			FString::Format(TEXT("Missing binary operation '{0}' between '{1}' and '{2}' type."), { BinaryOperation, LeftTypeName, RightTypeName }),
			EFormulaInvokeErrorCode::MissingBinaryOperation
		);
	}
	static FFormulaInvokeError CanCreateInstanceOfAbstractClass(const FString& TypeName)
	{
		return FFormulaInvokeError(
			FString::Format(TEXT("Can't create instance of abstract '{0}' class."), { TypeName }),
			EFormulaInvokeErrorCode::CanCreateInstanceOfAbstractClass
		);
	}
};