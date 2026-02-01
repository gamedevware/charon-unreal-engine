// Copyright GameDevWare, Denis Zykov 2025

#include "FFormulaUnrealType.h"
#include "GameData/Formulas/FFormulaFunction.h"

static TArray<FString> NoMembers;
FFormulaUnrealType::FFormulaUnrealType(UStruct* StructPtr)
	: StructOrClassPtr(StructPtr)
{
	check(StructPtr);
}

static bool IsStatic(const UFunction* Function)
{
	check(Function);

	return (Function->FunctionFlags & EFunctionFlags::FUNC_Static) != 0;
}

bool FFormulaUnrealType::IsAssignableFrom(UField* Type) const
{
	const UStruct* OtherStruct = Cast<UStruct>(Type);
	return OtherStruct && OtherStruct->IsChildOf(this->StructOrClassPtr.Get());
}

FString FFormulaUnrealType::GetCPPType() const
{
	const UStruct* Struct = this->StructOrClassPtr.Get();
	return Struct->GetPrefixCPP() + Struct->GetName();
}

const TArray<FString>& FFormulaUnrealType::GetPropertyNames(bool bStatic)
{
	if (bStatic && !this->IsClass())
	{
		return NoMembers;
	}

	const UStruct* Struct = this->StructOrClassPtr.Get();
	if (!this->PropertyNames.IsValid())
	{
		this->PropertyNames = MakeShared<TArray<FString>>();

		for (TFieldIterator<FProperty> It(Struct); It; ++It)
		{
			const FProperty* Property = *It;
			this->PropertyNames->Add(Property->GetName());
		}
	}
	return *this->PropertyNames;
}

const TArray<FString>& FFormulaUnrealType::GetFunctionNames(bool bStatic)
{
	const UClass* DeclaringClass = Cast<UClass>(this->StructOrClassPtr.Get());
	if (bStatic)
	{
		if (!this->StaticFunctionNames.IsValid())
		{
			this->StaticFunctionNames = MakeShared<TArray<FString>>();

			for (TFieldIterator<UFunction> It(DeclaringClass); It; ++It)
			{
				if (const UFunction* Function = *It; IsStatic(Function))
				{
					this->StaticFunctionNames->Add(Function->GetName());
				}
			}
		}
		return *this->StaticFunctionNames;
	}
	else
	{
		if (!this->FunctionNames.IsValid())
		{
			this->FunctionNames = MakeShared<TArray<FString>>();

			for (TFieldIterator<UFunction> It(DeclaringClass); It; ++It)
			{
				if (const UFunction* Function = *It; !IsStatic(Function))
				{
					this->FunctionNames->Add(Function->GetName());
				}
			}
		}
		return *this->FunctionNames;
	}
}

bool FFormulaUnrealType::TryGetFunction(const FString& MemberName, bool bStatic,
                                               const FFormulaFunction*& FoundFunction)
{
	UClass* DeclaringClass = Cast<UClass>(this->StructOrClassPtr.Get());
	if (bStatic)
	{
		if (!this->StaticFunctions.IsValid())
		{
			this->StaticFunctions = MakeShared<TMap<FString, FFormulaFunction>>();

			for (TFieldIterator<UFunction> It(DeclaringClass, EFieldIterationFlags::IncludeAll); It; ++It)
			{
				if (const auto Function = *It; IsStatic(Function))
				{
					this->StaticFunctions->Add(Function->GetName(), FFormulaFunction(Function, DeclaringClass, /*bUseClassDefaultObject*/ true));
				}
			}
		}
		FoundFunction = this->StaticFunctions->Find(MemberName);
		return FoundFunction != nullptr;
	}
	else
	{
		if (!this->Functions.IsValid())
		{
			this->Functions = MakeShared<TMap<FString, FFormulaFunction>>();

			for (TFieldIterator<UFunction> It(DeclaringClass); It; ++It)
			{
				if (const auto Function = *It; !IsStatic(Function))
				{
					this->Functions->Add(Function->GetName(), FFormulaFunction(Function, DeclaringClass, /*bUseClassDefaultObject*/ false));
				}
			}
		}
		FoundFunction = this->Functions->Find(MemberName);
		return FoundFunction != nullptr;
	}
}

bool FFormulaUnrealType::TryGetProperty(const FString& MemberName, bool bStatic,
                                               const FFormulaProperty*& FoundProperty)
{
	if (!this->IsClass() && bStatic)
	{
		return false;
	}

	UStruct* Struct = this->StructOrClassPtr.Get();
	if (bStatic)
	{
		if (!this->StaticProperties.IsValid())
		{
			this->StaticProperties = MakeShared<TMap<FString, FFormulaProperty>>();
			for (TFieldIterator<FProperty> It(Struct); It; ++It)
			{
				const auto Property = *It;
				this->StaticProperties->Add(
					Property->GetName(), FFormulaProperty(Property, Struct, /*bUseClassDefaultObject*/ true));
			}
		}
		FoundProperty = this->StaticProperties->Find(MemberName);
		return FoundProperty != nullptr;
	}
	else
	{
		if (!this->Properties.IsValid())
		{
			this->Properties = MakeShared<TMap<FString, FFormulaProperty>>();
			for (TFieldIterator<FProperty> It(Struct); It; ++It)
			{
				const auto Property = *It;
				this->Properties->Add(Property->GetName(), FFormulaProperty(Property, Struct, /*bUseClassDefaultObject*/ false));
			}
		}
		FoundProperty = this->Properties->Find(MemberName);
		return FoundProperty != nullptr;
	}
}
