// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "FFormulaExecutionResult.h"
#include "FFormulaExecutionResult.h"
#include "Misc/TVariant.h"

enum class EFormulaExecutionErrorCode
{
	Unknown,
	Exec_ExpressionIsInvalid,
	Exec_NullReference,
	Exec_UnsupportedExpression,
	Bind_MethodSignatureMismatch,
	Bind_UnsupportedObjectType,
	Bind_InvalidArraySize,
	Bind_StaticMemberNotFound,
	Bind_MemberNotFound,
	Bind_InvalidConditional,
	Bind_InvalidMapKey,
	Bind_MemberAccessFailed,
	Coll_KeyNotFound,
	Coll_IndexOutOfBounds,
	Coll_AddFailed,
	Op_BinaryNotSupported,
	Op_UnaryNotSupported,
	Op_IndexerNotFound,
	Type_UnableToResolveGlobal,
	Type_AbstractInstantiation,
	Type_UnsupportedCollectionType,
	Type_InvalidArrayType,
	Type_Unresolved,
	Type_NoConversion,
	Type_NullConversionIllegal,
	Type_InvalidCast,
	Type_UnsolicitedStructType,
};

class CHARON_API FFormulaExecutionError
{
public:
	const FString Message;
	const EFormulaExecutionErrorCode Code;

	FFormulaExecutionError(const FString& InMessage, const EFormulaExecutionErrorCode InCode)
		: Message(InMessage), Code(InCode)
	{
	}

	static FFormulaExecutionError InvalidCastError(const FString& FromTypeName, const FString& ToTypeName)
	{
		return FFormulaExecutionError(
			FString::Format(TEXT("Cannot cast object of type '{0}' to '{1}'."), {FromTypeName, ToTypeName}),
			EFormulaExecutionErrorCode::Type_InvalidCast
		);
	}

	static FFormulaExecutionError CanConvertNullToType(const FString& ToTypeName)
	{
		return FFormulaExecutionError(
			FString::Format(TEXT("Cannot convert null to type '{0}'."), {ToTypeName}),
			EFormulaExecutionErrorCode::Type_NullConversionIllegal
		);
	}

	static FFormulaExecutionError CantConvertToType(const FString& FromTypeName, const FString& ToTypeName)
	{
		return FFormulaExecutionError(
			FString::Format(TEXT("No conversion exists from type '{0}' to '{1}'."), {FromTypeName, ToTypeName}),
			EFormulaExecutionErrorCode::Type_NoConversion
		);
	}

	static FFormulaExecutionError InvalidConditionResultType(const FString& TypeName)
	{
		return FFormulaExecutionError(
			FString::Format(TEXT("Conditional expression result cannot be of type '{0}'. Boolean or compatible type expected."), {TypeName}),
			EFormulaExecutionErrorCode::Bind_InvalidConditional
		);
	}

	static FFormulaExecutionError UnableToResolveType(const FString& TypeName)
	{
		return FFormulaExecutionError(
			FString::Format(TEXT("Failed to resolve type '{0}'. Ensure the type is registered in the known types list."), {TypeName}),
			EFormulaExecutionErrorCode::Type_Unresolved
		);
	}

	static FFormulaExecutionError CantFindIndexer(const FString& ListType, const FString& ArgumentTypes)
	{
		return FFormulaExecutionError(
			FString::Format(TEXT("Type '{0}' does not define an indexer accepting arguments: ({1})."), {ListType, ArgumentTypes}),
			EFormulaExecutionErrorCode::Op_IndexerNotFound
		);
	}

	static FFormulaExecutionError InvalidDictionaryKeyType(const FString& KeyValue, const FString& KeyType,
	                                                       const FString& ExpectedKeyType)
	{
		return FFormulaExecutionError(
			FString::Format(TEXT("Invalid map key '{0}' of type '{1}'. Expected key type: '{2}'."), {KeyValue, KeyType, ExpectedKeyType}),
			EFormulaExecutionErrorCode::Bind_InvalidMapKey
		);
	}

	static FFormulaExecutionError IndexOutOfRange(int32 Index, int32 CollectionCount)
	{
		return FFormulaExecutionError(
			FString::Format(TEXT("Index {0} is out of bounds for collection of size {1}."), {Index, CollectionCount}),
			EFormulaExecutionErrorCode::Coll_IndexOutOfBounds
		);
	}

	static FFormulaExecutionError DictionaryKeyNotFound(const FString& KeyValue, const FString& KeyType)
	{
		return FFormulaExecutionError(
			FString::Format(TEXT("Key '{0}' (Type: {1}) not found in map."), {KeyValue, KeyType}),
			EFormulaExecutionErrorCode::Coll_KeyNotFound
		);
	}

	static FFormulaExecutionError NullReference()
	{
		return FFormulaExecutionError(TEXT("Attempted to access a null object reference."),
			EFormulaExecutionErrorCode::Exec_NullReference
		);
	}

	static FFormulaExecutionError CantFindMember(const FString& TypeName, const FString& MemberName,
	                                             const FString& AllMemberNames)
	{
		return FFormulaExecutionError(
			FString::Format(TEXT("Cannot find member '{1}' on type '{0}'. Available members: {2}."), {TypeName, MemberName, AllMemberNames}),
			EFormulaExecutionErrorCode::Bind_MemberNotFound
		);
	}

	static FFormulaExecutionError MemberAccessFailed(const FString& TypeName, const FString& MemberName)
	{
		return FFormulaExecutionError(
			FString::Format(TEXT("Failed to access member '{1}' on type '{0}'."), {TypeName, MemberName}),
			EFormulaExecutionErrorCode::Bind_MemberAccessFailed
		);
	}

	static FFormulaExecutionError CantFindStaticMember(const FString& TypeName, const FString& MemberName,
	                                                   const FString& AllMemberNames)
	{
		return FFormulaExecutionError(
			FString::Format(TEXT("Cannot find static member '{1}' on type '{0}'. Available static members: {2}."), {TypeName, MemberName, AllMemberNames}),
			EFormulaExecutionErrorCode::Bind_StaticMemberNotFound
		);
	}

	static FFormulaExecutionError ExpressionIsInvalid()
	{
		return FFormulaExecutionError(
			TEXT("FFormulaExpression is in an invalid or uninitialized state."),
			EFormulaExecutionErrorCode::Exec_ExpressionIsInvalid
		);
	}

	static FFormulaExecutionError UnableToResolveGlobalName(const FString& MemberName, const FString& AllGlobalMembers)
	{
		return FFormulaExecutionError(
			FString::Format(TEXT("Cannot resolve global member or argument '{0}'. Available globals: {1}."), {MemberName, AllGlobalMembers}),
			EFormulaExecutionErrorCode::Type_UnableToResolveGlobal
		);
	}

	static FFormulaExecutionError InvalidArrayType(const FString& TypeName)
	{
		return FFormulaExecutionError(
			FString::Format(TEXT("Invalid array declaration '{0}'. Arrays must be 'Array' with exactly one element type argument."), {TypeName}),
			EFormulaExecutionErrorCode::Type_InvalidArrayType
		);
	}

	static FFormulaExecutionError UnsupportedCollectionType(const FString& TypeName)
	{
		return FFormulaExecutionError(
			FString::Format(TEXT("Collection type '{0}' is unsupported. Only UObject-derived types and basic structures are allowed."), {TypeName}),
			EFormulaExecutionErrorCode::Type_UnsupportedCollectionType
		);	
	}
	
	static FFormulaExecutionError MissingContextForStruct(const FString& TypeName)
	{
		return FFormulaExecutionError(
			FString::Format(TEXT("Cannot create a struct of type '{0}' because it lacks a compatible destination. Structs must be passed into a function or assigned to a property for proper memory management."), {TypeName}),
			EFormulaExecutionErrorCode::Type_UnsolicitedStructType
		);	
	}

	static FFormulaExecutionError InvalidArraySizeValue(const FString& SizeValueTypeName)
	{
		return FFormulaExecutionError(
			FString::Format(TEXT("Invalid collection size type '{0}'. Expected an integer value."), {SizeValueTypeName}),
			EFormulaExecutionErrorCode::Bind_InvalidArraySize
		);
	}

	static FFormulaExecutionError UnsupportedObjectType(const FString& TypeName)
	{
		return FFormulaExecutionError(
			FString::Format(TEXT("Type '{0}' cannot be constructed. Only UCLASS reflection-visible types and primitives are supported."), {TypeName}),
			EFormulaExecutionErrorCode::Bind_UnsupportedObjectType
		);
	}

	static FFormulaExecutionError UnableToBindMethodToParameters(const FString& TypeName, const FString& FunctionName,
	                                                             const FString& ArgumentTypes, int32 ArgumentCount)
	{
		const FString DetailedMessage = (ArgumentCount > 0) ?
			FString::Format(TEXT("Cannot bind method '{1}' on type '{0}' with ({2}) arguments."), { TypeName, FunctionName, ArgumentTypes }) :
			FString::Format(TEXT("Cannot bind method '{1}' on type '{0}' with no arguments."),{ TypeName, FunctionName });

		return FFormulaExecutionError(DetailedMessage, EFormulaExecutionErrorCode::Bind_MethodSignatureMismatch);
	}

	static FFormulaExecutionError MissingUnaryOperation(const FString& TypeName, const FString& UnaryOperation)
	{
		return FFormulaExecutionError(
			FString::Format(TEXT("Unary operator '{0}' is not defined for type '{1}'."), {UnaryOperation, TypeName}),
			EFormulaExecutionErrorCode::Op_UnaryNotSupported
		);
	}

	static FFormulaExecutionError MissingBinaryOperation(const FString& LeftTypeName, const FString& RightTypeName,
	                                                     const FString& BinaryOperation)
	{
		return FFormulaExecutionError(
			FString::Format(TEXT("Binary operator '{0}' is not defined between types '{1}' and '{2}'."), {BinaryOperation, LeftTypeName, RightTypeName}),
			EFormulaExecutionErrorCode::Op_BinaryNotSupported
		);
	}

	static FFormulaExecutionError CanCreateInstanceOfAbstractClass(const FString& TypeName)
	{
		return FFormulaExecutionError(
			FString::Format(TEXT("Cannot instantiate abstract class '{0}'."), {TypeName}),
			EFormulaExecutionErrorCode::Type_AbstractInstantiation
		);
	}
	static FFormulaExecutionError UnsupportedExpression(const FString& ExpressionTypeName)
	{
		return FFormulaExecutionError(
			FString::Format(TEXT("Expression type '{0}' is not supported."), {ExpressionTypeName}),
			EFormulaExecutionErrorCode::Exec_UnsupportedExpression
		);
	}
	static FFormulaExecutionError CollectionAddFailed(const FString& CollectionTypeName, const FString& AddedElementType)
	{
		return FFormulaExecutionError(
			FString::Format(TEXT("Failed to add ({1}) tuple to '{0}' collection."), {CollectionTypeName, AddedElementType}),
			EFormulaExecutionErrorCode::Coll_AddFailed
		);
	}
};
