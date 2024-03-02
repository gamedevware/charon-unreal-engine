// Copyright GameDevWare, Denis Zykov 2024

#pragma once

enum class EDocumentSortDirection
{
	Ascending,
	Descending,
};

struct FDocumentSorter
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
