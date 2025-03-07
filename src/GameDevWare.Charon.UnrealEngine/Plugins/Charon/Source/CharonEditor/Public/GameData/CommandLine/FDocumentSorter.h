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
