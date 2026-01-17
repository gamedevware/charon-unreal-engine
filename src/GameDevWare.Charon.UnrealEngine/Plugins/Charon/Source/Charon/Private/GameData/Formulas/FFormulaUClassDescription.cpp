#include "FFormulaUClassDescription.h"
#include "GameData/Formulas/FFormulaMethodGroup.h"

static TArray<FString> NoMembers;

FFormulaUClassDescription::FFormulaUClassDescription(UClass* ClassPtr)
	: ClassPtr(ClassPtr) { }


FString GetMemberName(const FField* Field)
{
#if WITH_METADATA	
	static const FName MetaTag = TEXT("FormulaMemberName");
	
	// Check if the custom meta tag exists
	if (Field->HasMetaData(MetaTag))
	{
		return Field->GetMetaData(MetaTag);
	}

	// Fallback to the actual C++ name
	return Field->GetName();
#else
	return FString();
#endif
}
FString GetMemberName(const UField* Field)
{
#if WITH_METADATA	
	static const FName MetaTag = TEXT("FormulaMemberName");

	// Check if the custom meta tag exists
	if (Field->HasMetaData(MetaTag))
	{
		return Field->GetMetaData(MetaTag);
	}

	// Fallback to the actual C++ name
	return Field->GetName();
#else
	return FString();
#endif
}

bool FFormulaUClassDescription::IsAssignableFrom(UStruct* Type) const
{
	return Type->IsChildOf(this->ClassPtr.Get());
}

FString FFormulaUClassDescription::GetCPPType() const
{
	return this->ClassPtr->GetPrefixCPP() + this->ClassPtr->GetName();
}

const TArray<FString>& FFormulaUClassDescription::GetPropertyNames(bool bStatic)
{
	if (bStatic || !this->ClassPtr)
	{
		return NoMembers;
	}

	if (!this->ClassPropertyNames.IsSet())
	{
		 TArray<FString> Names;
		
		for (TFieldIterator<FProperty> It(this->ClassPtr.Get()); It; ++It)
		{
			FProperty* Property = *It;
        
			// Get the name of the property
			FString PropertyName = GetMemberName(Property);
			Names.Add(PropertyName);
		}
		this->ClassPropertyNames = Names;
	}

	return this->ClassPropertyNames.GetValue();
}

const TArray<FString>& FFormulaUClassDescription::GetFunctionNames(bool bStatic)
{
	if (bStatic || !this->ClassPtr)
	{
		return NoMembers;
	}

	if (!this->ClassFunctionNames.IsSet())
	{
		TArray<FString> Names;
		
		for (TFieldIterator<UFunction> It(this->ClassPtr.Get()); It; ++It)
		{
			UFunction* Function = *It;
        
			// Get the name of the function
			FString FuncName = GetMemberName(Function);
			
			Names.Add(FuncName);
		}
		this->ClassFunctionNames = Names;
	}

	return this->ClassFunctionNames.GetValue();
}

bool FFormulaUClassDescription::TryGetBinaryOperation(EBinaryOperationType Operation, FFormulaMethodGroup*& FoundOperations)
{
	return false;
}

bool FFormulaUClassDescription::TryGetUnaryOperation(EUnaryOperationType Operation, FFormulaMethodGroup*& FoundMOperations)
{
	return false;
}

bool FFormulaUClassDescription::TryGetMethod(const FString& MemberName, bool bStatic, FFormulaMethodGroup*& FoundMember)
{
	if (bStatic || !this->ClassPtr)
	{
		return false;
	}
	
	if (FFormulaMethodGroup* BoundMember = this->BoundFunctions.Find(MemberName))
	{
		FoundMember = BoundMember;
		return true;
	}

	FFormulaMethodGroup& NewBoundMember = this->BoundFunctions.Emplace(MemberName);
	for (TFieldIterator<UFunction> It(this->ClassPtr.Get()); It; ++It)
	{
		if (const auto Function = *It; GetMemberName(Function) == MemberName)
        {
	        NewBoundMember.AddFunction(Function);
        }
	}

	FoundMember = &NewBoundMember;
	return true;
}

bool FFormulaUClassDescription::TryGetProperty(const FString& MemberName, bool bStatic, FProperty*& FoundMember)
{
	
	if (bStatic || !this->ClassPtr)
	{
		return false;
	}
	
	if (const TFieldPath<FProperty>* BoundMember = this->BoundProperties.Find(MemberName))
	{
		FoundMember = BoundMember ? BoundMember->Get() : nullptr;  
		return !!FoundMember;
	}

	FoundMember = nullptr;
	for (TFieldIterator<FProperty> It(this->ClassPtr.Get()); It; ++It)
	{
		if (const auto Property = *It; GetMemberName(Property) == MemberName)
		{
			FoundMember = Property;
			return true;
		}
	}
	return false;
}

bool FFormulaUClassDescription::TryGetIndexer(int Rank, FFormulaMethodGroup*& FoundMember)
{
	return false;
}

bool FFormulaUClassDescription::TryGetConversionOperation(FFormulaMethodGroup*& FoundMember)
{
	return false;
}

