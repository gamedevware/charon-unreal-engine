// Copyright GameDevWare, Denis Zykov 2025

#pragma once

enum class EDocumentFilterOperation
{
	GreaterThan,
	GreaterThanOrEqual,
	LessThan,
	LessThanOrEqual,
	Equal,
	NotEqual,
	Like,
	In
};

struct CHARONEDITOR_API FDocumentFilter
{
public:
	FString PropertyName;
	EDocumentFilterOperation Operation;
	FString Value;

	FDocumentFilter(const FString& PropertyName, const EDocumentFilterOperation Operation, const FString& Value)
		:PropertyName(PropertyName), Operation(Operation), Value(Value)
	{
		
	}
	
	FString GetOperationName() const
	{
		switch (Operation)
		{
		case EDocumentFilterOperation::GreaterThan: return TEXT("GreaterThan");
		case EDocumentFilterOperation::GreaterThanOrEqual: return TEXT("GreaterThanOrEqual");
		case EDocumentFilterOperation::LessThan: return TEXT("LessThan");
		case EDocumentFilterOperation::LessThanOrEqual: return TEXT("LessThanOrEqual");
		case EDocumentFilterOperation::Equal: return TEXT("Equal");
		case EDocumentFilterOperation::NotEqual: return TEXT("NotEqual");
		case EDocumentFilterOperation::Like: return TEXT("Like");
		case EDocumentFilterOperation::In: return TEXT("In");
		default: return TEXT("INVALID_OPERATION");
		}
	}

	FString GetValueQuoted()
	{
		const bool bIsQuoted = Value.StartsWith("\"") && Value.EndsWith("\"");
		const bool bHasInvalidChars = !bIsQuoted && (Value.Find(" ") >= 0 || Value.Find("\"") > 0); 
		
		if (Value.IsEmpty())
		{
			return TEXT("\"\"");
		}
		else if (bHasInvalidChars)
		{
			return FString::Format(TEXT("\"{0}\""), { Value.Replace(TEXT("\""), TEXT("\"\"")) });
		}
		else
		{
			return Value;
		}
		
	}
};
