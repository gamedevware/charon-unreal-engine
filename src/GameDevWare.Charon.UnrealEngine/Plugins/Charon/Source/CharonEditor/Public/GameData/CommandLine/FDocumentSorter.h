// Copyright GameDevWare, Denis Zykov 2025

#pragma once

enum class EDocumentSortDirection
{
	Ascending,
	Descending,
};

struct CHARONEDITOR_API FDocumentSorter
{
public:
	FString PropertyName;
	EDocumentSortDirection Direction;

	FDocumentSorter(const FString& PropertyName, const EDocumentSortDirection Direction)
		:PropertyName(PropertyName), Direction(Direction)
	{
		
	}
	
	FString GetDirectionName() const
	{
		switch (Direction)
		{
			case EDocumentSortDirection::Ascending: return TEXT("ASC");
			case EDocumentSortDirection::Descending: return TEXT("DESC");
			default: return TEXT("UNKNOWN_DIRECTION");
		}
	}
};
