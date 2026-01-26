// Copyright GameDevWare, Denis Zykov 2025

#include "GameData/Formulas/Expressions/FNewExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/IFormulaType.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"

DEFINE_LOG_CATEGORY(LogNewExpression);

FNewExpression::FNewExpression(const TSharedRef<FJsonObject>& ExpressionObj) :
	Arguments(FExpressionBuildHelper::GetArguments(ExpressionObj, FFormulaNotation::ARGUMENTS_ATTRIBUTE)),
	NewObjectType(FExpressionBuildHelper::GetTypeRef(ExpressionObj, FFormulaNotation::TYPE_ATTRIBUTE))
{
}

FNewExpression::FNewExpression(const TSharedPtr<FFormulaTypeReference>& NewObjectType,
	const TMap<FString, TSharedPtr<FFormulaExpression>>& Arguments) :
	Arguments(Arguments), NewObjectType(NewObjectType)
{
}

FFormulaExecutionResult FNewExpression::Execute(const FFormulaExecutionContext& Context, FProperty* ExpectedType) const
{
	if (!this->IsValid())
	{
		return FFormulaExecutionError::ExpressionIsInvalid();
	}

	if (this->Arguments.Num() != 0)
	{
		UE_LOG(LogNewExpression, Error, TEXT("'%s': 'new' expression cannot take constructor arguments (found %d)."),
			   *this->NewObjectType->GetFullName(true), this->Arguments.Num());
		return FFormulaExecutionError::ExpressionIsInvalid();
	}

	const auto NewObjectTypeRef = Context.TypeResolver->GetTypeDescription(this->NewObjectType);
	if (!NewObjectTypeRef.IsValid())
	{
		return FFormulaExecutionError::UnableToResolveType(this->NewObjectType->GetFullName(/* include generics */ true));
	}

	const UField* ClassOrStructPtr = NewObjectTypeRef->GetTypeClassOrStruct();
	if (const UClass* NewObjectClass = Cast<UClass>(ClassOrStructPtr))
	{
		if ((NewObjectClass->ClassFlags & EClassFlags::CLASS_Abstract) != 0)
		{
			return FFormulaExecutionError::CanCreateInstanceOfAbstractClass(this->NewObjectType->GetFullName(/* include generics */ true));
		}
		
		UObject* NewObjectPtr = NewObject<UObject>(GetTransientPackage(), NewObjectClass);
		return NewObjectPtr;
	}

	else if (Cast<UScriptStruct>(ClassOrStructPtr))
	{
		/*
		 * TODO: FFormulaValue cant hold arbitrary structs, so no struct construction in formulas right now, except well known like time, fstring, name etc
		*/
		/*
		TArray<uint8> StructData;
		StructData.SetNumZeroed(ExpectedType->GetStructureSize());
		uint8* StructMemory = StructData.GetData();
		NewStructType->InitializeStruct(StructMemory);
		
		// return  )
		// NewStructType->DestroyStruct(StructMemory);
		*/
		return FFormulaExecutionError::UnsupportedObjectType(NewObjectTypeRef->GetCPPType());
	}

	
	switch (NewObjectTypeRef->GetTypeCode()) {
	case EFormulaValueType::Null: return FFormulaValue::Null();
	case EFormulaValueType::Boolean: return FFormulaValue::FalseBool();
	case EFormulaValueType::UInt8: return MakeShared<FFormulaValue>(static_cast<uint8>(0));
	case EFormulaValueType::UInt16: return MakeShared<FFormulaValue>(static_cast<uint16>(0));
	case EFormulaValueType::UInt32: return MakeShared<FFormulaValue>(static_cast<uint32>(0));
	case EFormulaValueType::UInt64: return MakeShared<FFormulaValue>(static_cast<uint64>(0));
	case EFormulaValueType::Int8: return MakeShared<FFormulaValue>(static_cast<int8>(0));
	case EFormulaValueType::Int16: return MakeShared<FFormulaValue>(static_cast<int16>(0));
	case EFormulaValueType::Int32: return MakeShared<FFormulaValue>(static_cast<int32>(0));
	case EFormulaValueType::Int64: return MakeShared<FFormulaValue>(static_cast<int64>(0));
	case EFormulaValueType::Float: return MakeShared<FFormulaValue>(static_cast<float>(0));
	case EFormulaValueType::Double: return MakeShared<FFormulaValue>(static_cast<double>(0));
	case EFormulaValueType::Timespan: return FTimespan();
	case EFormulaValueType::DateTime: return FDateTime();
	case EFormulaValueType::String: return FString();
	case EFormulaValueType::Name: return FName();
	case EFormulaValueType::Text: return FText();
	case EFormulaValueType::ObjectPtr: return FFormulaValue::Null();
	case EFormulaValueType::Enum:
	case EFormulaValueType::Struct:
	default:
		return FFormulaExecutionError::UnsupportedObjectType(NewObjectTypeRef->GetCPPType());
	}
}

bool FNewExpression::IsValid() const
{
	if (!this->NewObjectType.IsValid())
	{
		return false;
	}
	for (auto ArgumentPair : this->Arguments)
	{
		if (!ArgumentPair.Value.IsValid())
		{
			return false;
		}
	}
	return true;
}

void FNewExpression::DebugPrintTo(FString& OutValue) const
{
	OutValue.Append("new ");
	if (this->NewObjectType.IsValid())
	{
		OutValue.Append(this->NewObjectType->GetFullName(true));
	}
	else
	{
		OutValue.Append(TEXT("#INVALID#"));
	}
	OutValue.Append("(");
	bool bFirstArgument = true;;
	for (auto ArgumentPair : this->Arguments)
	{
		if (!bFirstArgument)
		{
			OutValue.Append(", ");
		}
		bFirstArgument = false;

		const bool bIsNumberKey = ArgumentPair.Key == TEXT("0") || FCString::Atoi(*ArgumentPair.Key) != 0;
		if (!bIsNumberKey)
		{
			OutValue.Append(ArgumentPair.Key).Append(": ");
		}
		if (ArgumentPair.Value.IsValid())
		{
			ArgumentPair.Value->DebugPrintTo(OutValue);
		}
		else
		{
			OutValue.Append(TEXT("#INVALID#"));
		}
	}
	OutValue.Append(")");
}
