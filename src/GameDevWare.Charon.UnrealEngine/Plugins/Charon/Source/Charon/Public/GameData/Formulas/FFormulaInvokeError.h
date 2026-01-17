#pragma once
#include "Misc/TVariant.h"

enum class EFormulaInvokeErrorCode
{
	InvalidCastError, CantConvertToType, CanConvertNullToType, InvalidConditionResultType, UnableToResolveType
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

};