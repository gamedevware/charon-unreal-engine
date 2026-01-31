// Copyright GameDevWare, Denis Zykov 2025

#include "GameData/Formulas/Expressions/FNewExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/IFormulaType.h"
#include "GameData/Formulas/Expressions/FNewArrayBoundExpression.h"
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

	FString NewObjectTypeFullName = this->NewObjectType->GetFullName(/*bWriteGenerics*/ false);
	if ((NewObjectTypeFullName == "Array" || NewObjectTypeFullName == "System.Array") && 
		this->NewObjectType->TypeArguments.Num() == 1)
	{
		return CreateNewArray(this->NewObjectType->TypeArguments[0], Context, ExpectedType);
	}
	else if ((NewObjectTypeFullName == "HashSet" || NewObjectTypeFullName == "System.HashSet" || NewObjectTypeFullName == "Set") && 
		this->NewObjectType->TypeArguments.Num() == 1)
	{
		return CreateNewSet(this->NewObjectType->TypeArguments[0], Context, ExpectedType);
	}
	else if ((NewObjectTypeFullName == "Dictionary" || NewObjectTypeFullName == "System.Collections.Dictionary" || NewObjectTypeFullName == "Map") && 
		this->NewObjectType->TypeArguments.Num() == 2)
	{
		return CreateNewMap(this->NewObjectType->TypeArguments[0], this->NewObjectType->TypeArguments[1], Context, ExpectedType);
	}
	
	const auto NewObjectTypeInfo = Context.TypeResolver->FindType(this->NewObjectType);
	if (!NewObjectTypeInfo.IsValid())
	{
		return FFormulaExecutionError::UnableToResolveType(this->NewObjectType->GetFullName(/* include generics */ true));
	}

	const UField* ClassOrStructPtr = NewObjectTypeInfo->GetTypeClassOrStruct();
	if (const UClass* NewObjectClass = Cast<UClass>(ClassOrStructPtr))
	{
		if ((NewObjectClass->ClassFlags & EClassFlags::CLASS_Abstract) != 0)
		{
			return FFormulaExecutionError::CanCreateInstanceOfAbstractClass(this->NewObjectType->GetFullName(/* include generics */ true));
		}
		
		UObject* NewObjectPtr = NewObject<UObject>(GetTransientPackage(), NewObjectClass);
		return NewObjectPtr;
	}
	else if (FStructProperty* StructProperty = CastField<FStructProperty>(ExpectedType))
	{
		if (StructProperty && NewObjectTypeInfo->IsAssignableFrom(StructProperty->Struct))
		{
			return MakeShared<FFormulaValue>(ExpectedType);
		}
		else
		{
			return FFormulaExecutionError::MissingContextForStruct(this->NewObjectType->GetFullName(/* include generics */ true));
		}
	}
	
	return CreatePrimitiveObject(NewObjectTypeInfo);
}


FFormulaExecutionResult FNewExpression::CreatePrimitiveObject(const TSharedPtr<IFormulaType>& NewObjectTypeRef) const
{
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
		return FFormulaExecutionError::UnsupportedObjectType(this->NewObjectType->GetFullName(/* include generics */ true));
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

bool FNewExpression::TryCreateArray(
	const TSharedPtr<IFormulaType>& ElementType, 
	FArrayProperty* ArrayProperty,
	TSharedPtr<FFormulaValue>& OutCreatedArray)
{
	TArray<uint8> Array;

	if (ArrayProperty && !ElementType->IsAssignableFrom(ArrayProperty->Inner, /*bWithoutConversion*/ true))
	{
		ArrayProperty = nullptr; // not compatible
	}
	
	if (!ArrayProperty)
	{
		switch (ElementType->GetTypeCode()) {
		case EFormulaValueType::Boolean: ArrayProperty = UDotNetBoolean::GetArrayProperty(); break;
		case EFormulaValueType::UInt8: ArrayProperty = UDotNetUInt8::GetArrayProperty(); break;
		case EFormulaValueType::UInt16: ArrayProperty = UDotNetUInt16::GetArrayProperty(); break;
		case EFormulaValueType::UInt32: ArrayProperty = UDotNetUInt32::GetArrayProperty(); break;
		case EFormulaValueType::UInt64: ArrayProperty = UDotNetUInt64::GetArrayProperty(); break;
		case EFormulaValueType::Int8: ArrayProperty = UDotNetInt8::GetArrayProperty(); break;
		case EFormulaValueType::Int16: ArrayProperty = UDotNetInt16::GetArrayProperty(); break;
		case EFormulaValueType::Int32: ArrayProperty = UDotNetInt32::GetArrayProperty(); break;
		case EFormulaValueType::Int64: ArrayProperty = UDotNetInt64::GetArrayProperty(); break;
		case EFormulaValueType::Float: ArrayProperty = UDotNetSingle::GetArrayProperty(); break;
		case EFormulaValueType::Double: ArrayProperty = UDotNetDouble::GetArrayProperty(); break;
		case EFormulaValueType::Timespan: ArrayProperty = UDotNetTimeSpan::GetArrayProperty(); break;
		case EFormulaValueType::DateTime: ArrayProperty = UDotNetDateTime::GetArrayProperty(); break;
		case EFormulaValueType::String: ArrayProperty = UDotNetString::GetArrayProperty(); break;
		case EFormulaValueType::Name: ArrayProperty = UDotNetName::GetArrayProperty(); break;
		case EFormulaValueType::Text: ArrayProperty = UDotNetText::GetArrayProperty(); break;
		case EFormulaValueType::Null:
		case EFormulaValueType::ObjectPtr: ArrayProperty = UDotNetObject::GetArrayProperty(); break;
		case EFormulaValueType::Struct: 
		case EFormulaValueType::Enum: ArrayProperty = nullptr; break; // not supported
		}
	}

	if (!ArrayProperty)
	{
		return  false;
	}
	
	OutCreatedArray = MakeShared<FFormulaValue>(static_cast<FProperty*>(ArrayProperty));
	return true;
}

FFormulaExecutionResult FNewExpression::CreateNewArray(
	const TSharedPtr<FFormulaTypeReference>& ElementTypeRef,
	const FFormulaExecutionContext& Context, 
	FProperty* ExpectedType) const
{
	check(ElementTypeRef.IsValid());
	
	const auto ElementType = Context.TypeResolver->FindType(ElementTypeRef);
	if (!ElementType.IsValid())
	{
		return FFormulaExecutionError::UnableToResolveType(ElementTypeRef->GetFullName(/* include generics */ true));
	}

	if (TSharedPtr<FFormulaValue> ArrayValue = FFormulaValue::Null(); 
		TryCreateArray(ElementType, CastField<FArrayProperty>(ExpectedType), ArrayValue))
	{
		return ArrayValue;
	}
	
	return FFormulaExecutionError::MissingContextForStruct(this->NewObjectType->GetFullName(/* include generics */ true));
}

FFormulaExecutionResult FNewExpression::CreateNewSet(
	const TSharedPtr<FFormulaTypeReference>& ElementTypeRef, 
	const FFormulaExecutionContext& Context, 
	FProperty* ExpectedType) const
{
	check(ElementTypeRef.IsValid());
	
	const auto ElementType = Context.TypeResolver->FindType(ElementTypeRef);
	if (!ElementType.IsValid())
	{
		return FFormulaExecutionError::UnableToResolveType(ElementTypeRef->GetFullName(/* include generics */ true));
	}
	
	if (FSetProperty* SetProperty = CastField<FSetProperty>(ExpectedType); 
		SetProperty && ElementType->IsAssignableFrom(SetProperty->GetElementProperty(), /*bWithoutConversion*/ true))
	{
		return MakeShared<FFormulaValue>(static_cast<FProperty*>(SetProperty));
	}
	
	return FFormulaExecutionError::MissingContextForStruct(this->NewObjectType->GetFullName(/* include generics */ true));
}

FFormulaExecutionResult FNewExpression::CreateNewMap(
	const TSharedPtr<FFormulaTypeReference>& KeyTypeRef, 
	const TSharedPtr<FFormulaTypeReference>& ValueTypeRef, 
	const FFormulaExecutionContext& Context, 
	FProperty* ExpectedType) const
{
	check(KeyTypeRef.IsValid());
	check(ValueTypeRef.IsValid());
	
	const auto KeyType = Context.TypeResolver->FindType(KeyTypeRef);
	if (!KeyType.IsValid())
	{
		return FFormulaExecutionError::UnableToResolveType(KeyTypeRef->GetFullName(/* include generics */ true));
	}
	
	const auto ValueType = Context.TypeResolver->FindType(ValueTypeRef);
	if (!ValueType.IsValid())
	{
		return FFormulaExecutionError::UnableToResolveType(ValueTypeRef->GetFullName(/* include generics */ true));
	}
	
	if (FMapProperty* MapProperty = CastField<FMapProperty>(ExpectedType); 
		MapProperty && KeyType->IsAssignableFrom(MapProperty->GetKeyProperty(), /*bWithoutConversion*/ true) && 
		ValueType->IsAssignableFrom(MapProperty->GetValueProperty(), /*bWithoutConversion*/ true))
	{
		return MakeShared<FFormulaValue>(static_cast<FProperty*>(MapProperty));
	}
	
	return FFormulaExecutionError::MissingContextForStruct(this->NewObjectType->GetFullName(/* include generics */ true));
}
