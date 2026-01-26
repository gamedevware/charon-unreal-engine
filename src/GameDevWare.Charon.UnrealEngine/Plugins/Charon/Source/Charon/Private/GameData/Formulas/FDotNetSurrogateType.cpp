// Copyright GameDevWare, Denis Zykov 2025

#include "FDotNetSurrogateType.h"

FName SelfParameterName = FName(TEXT("Self"));

FDotNetSurrogateType::FDotNetSurrogateType(UClass* SurrogateClass):
	ClassPtr(SurrogateClass),
	LiteralField(*SurrogateClass->FindPropertyByName(TEXT("__Literal"))),
	TypeCode(GetPropertyTypeCode(SurrogateClass->FindPropertyByName(TEXT("__Literal"))))
{
}

bool FDotNetSurrogateType::CanBeNull() const
{
	return this->GetTypeCode() == EFormulaValueType::ObjectPtr;
}

bool FDotNetSurrogateType::IsAssignableFrom(UField* Type) const
{
	const UClass* OtherClass = Cast<UClass>(Type);
	return OtherClass && OtherClass == ClassPtr.Get();
}

EFormulaValueType FDotNetSurrogateType::GetTypeCode() const
{
	return TypeCode;
}

FString FDotNetSurrogateType::GetCPPType() const
{
	FString* ExtendedTypeText = nullptr;
	FString Name = this->LiteralField.GetCPPType(ExtendedTypeText);
	return ExtendedTypeText ? Name + *ExtendedTypeText : Name;
}

bool IsPublic(const UFunction* Function)
{
	return (Function->FunctionFlags & EFunctionFlags::FUNC_Public) != 0; 
}
bool IsPublic(const FProperty* Property)
{
	return (Property->PropertyFlags & EPropertyFlags::CPF_NativeAccessSpecifierPublic) != 0; 
}
bool IsExtensionMethod(const UFunction* Function, const FProperty* LiteralField)
{
	check(Function);
	check(LiteralField);
	
	for (TFieldIterator<FProperty> It(Function); It; ++It)
	{
		const FProperty* Argument = *It;
		if ((Argument->PropertyFlags & CPF_ReturnParm) != 0)
		{
			continue;
		}
		
		if (Argument->GetFName() == SelfParameterName && Argument->SameType(LiteralField))
		{
			return true;
		}
		break;
	}
	return false;
}

bool FDotNetSurrogateType::TryGetFunction(const FString& MemberName, bool bStatic,
                                          const FFormulaFunction*& FoundFunction)
{
	UClass* DeclaringClass = Cast<UClass>(this->ClassPtr.Get());
	if (bStatic)
	{
		if (!this->StaticFunctions.IsValid())
		{
			this->StaticFunctions = MakeShared<TMap<FString, FFormulaFunction>>();

			for (TFieldIterator<UFunction> It(DeclaringClass); It; ++It)
			{
				if (const auto Function = *It; IsPublic(Function) && !IsExtensionMethod(Function, &this->LiteralField))
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
				if (const auto Function = *It; IsPublic(Function) && IsExtensionMethod(Function, &this->LiteralField))
				{
					const auto ExtensionFunctionInvoker = FFormulaFunction::CreateExtensionFunctionInvoker(Function, DeclaringClass); 
					this->Functions->Add(Function->GetName(), FFormulaFunction(ExtensionFunctionInvoker, DeclaringClass, /*bUseClassDefaultObject*/ false));
				}
			}
		}
		FoundFunction = this->Functions->Find(MemberName);
		return FoundFunction != nullptr;
	}
}

const TArray<FString>& FDotNetSurrogateType::GetFunctionNames(bool bStatic)
{
	const UClass* DeclaringClass = Cast<UClass>(this->ClassPtr.Get());
	if (bStatic)
	{
		if (!this->StaticFunctionNames.IsValid())
		{
			this->StaticFunctionNames = MakeShared<TArray<FString>>();
			for (TFieldIterator<UFunction> It(DeclaringClass); It; ++It)
			{
				if (const auto Function = *It; IsPublic(Function) && !IsExtensionMethod(Function, &this->LiteralField))
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
				if (const auto Function = *It; IsPublic(Function) && IsExtensionMethod(Function, &this->LiteralField))
				{
					FunctionNames->Add(Function->GetName());
				}
			}
		}
		return *this->FunctionNames;
	}
}

bool IsExtensionProperty(const UFunction* Function, const FProperty* LiteralField)
{
	check(Function);
	check(LiteralField);

	const FString FunctionName = Function->GetName();
	int ParameterCount = 0;
	bool bHasSelfParameter = false;
	for (TFieldIterator<FProperty> It(Function); It; ++It)
	{
		const FProperty* Argument = *It;
		if ((Argument->PropertyFlags & CPF_ReturnParm) != 0)
		{
			continue;
		}
		
		if (Argument->GetFName() == SelfParameterName && Argument->SameType(LiteralField))
		{
			bHasSelfParameter = true;
		}
		ParameterCount++;
	}

	return (FunctionName.StartsWith(TEXT("Get")) && bHasSelfParameter && ParameterCount == 1) ||
		(FunctionName.StartsWith(TEXT("Set")) && bHasSelfParameter && ParameterCount == 2);
	
}

bool FDotNetSurrogateType::TryGetProperty(const FString& MemberName, bool bStatic, const FFormulaProperty*& FoundProperty)
{
	UClass* DeclaringClass = this->ClassPtr.Get();
	if (bStatic)
	{
		if (!this->StaticProperties.IsValid())
		{
			this->StaticProperties = MakeShared<TMap<FString, FFormulaProperty>>();
			for (TFieldIterator<FProperty> It(DeclaringClass); It; ++It)
			{
				if (const auto Property = *It; IsPublic(Property))
				{
					this->StaticProperties->Add(
						Property->GetName(), FFormulaProperty(Property, DeclaringClass, /*bUseClassDefaultObject*/ true));	
				}
				
			}
		}
		FoundProperty = this->StaticProperties->Find(MemberName);
		return FoundProperty != nullptr;
	}
	else
	{
		if (!this->Properties.IsValid())
		{
			TMap<FString, TTuple<UFunction*, UFunction*>> ExtensionProperties;
			for (TFieldIterator<UFunction> It(DeclaringClass); It; ++It)
			{
				if (const auto Function = *It; IsPublic(Function) && IsExtensionMethod(Function, &this->LiteralField))
				{
					FString FunctionName = Function->GetName();
					FString PropertyName = FunctionName.RightChop(FunctionName.Len() - 3);
					TTuple<UFunction*, UFunction*> GetterAndSetter = ExtensionProperties.FindOrAdd(PropertyName);
					if (FunctionName.StartsWith(TEXT("Get")))
					{
						GetterAndSetter = TTuple<UFunction*, UFunction*>(Function, GetterAndSetter.Value);
					}
					else
					{
						GetterAndSetter = TTuple<UFunction*, UFunction*>(GetterAndSetter.Key, Function);
					}
				}
			}

			this->Properties = MakeShared<TMap<FString, FFormulaProperty>>();
			for (const auto ExtensionProperty : ExtensionProperties)
			{
				FString PropertyName = ExtensionProperty.Key;
				UFunction* Getter = ExtensionProperty.Value.Key;
				UFunction* Setter = ExtensionProperty.Value.Value;

				FFormulaPropertyGetterFunc GetterFunc;
				if (Getter)
				{
					const auto GetterInvoker = FFormulaFunction::CreateExtensionFunctionInvoker(Getter, DeclaringClass);
					GetterFunc = FFormulaProperty::CreateGetterFromFunctionInvoker(GetterInvoker);
				}
				FFormulaPropertySetterFunc SetterFunc;
				if (Setter)
				{
					const auto SetterInvoker = FFormulaFunction::CreateExtensionFunctionInvoker(Setter, DeclaringClass);
					SetterFunc = FFormulaProperty::CreateSetterFromFunctionInvoker(SetterInvoker);
				}

				this->Properties->Add(MemberName, FFormulaProperty(Getter->GetReturnProperty(), GetterFunc, SetterFunc, DeclaringClass, /*bUseClassDefaultObject*/ false));
			}


		}
		FoundProperty = this->Properties->Find(MemberName);
		return FoundProperty != nullptr;
	}
}


const TArray<FString>& FDotNetSurrogateType::GetPropertyNames(bool bStatic)
{
	const UClass* DeclaringClass = this->ClassPtr.Get();
	if (bStatic)
	{
		if (!this->StaticPropertyNames.IsValid())
		{
			this->StaticPropertyNames = MakeShared<TArray<FString>>();
			for (TFieldIterator<FProperty> It(DeclaringClass); It; ++It)
			{
				if (const auto Property = *It; IsPublic(Property))
				{
					StaticPropertyNames->Add(Property->GetName());
				}
				
			}
		}
		return *StaticPropertyNames;
	}
	else
	{
		if (!this->PropertyNames.IsValid())
		{
			this->PropertyNames = MakeShared<TArray<FString>>();
			for (TFieldIterator<UFunction> It(DeclaringClass); It; ++It)
			{
				if (const auto Function = *It; IsPublic(Function) && IsExtensionMethod(Function, &this->LiteralField))
				{
					FString FunctionName = Function->GetName();
					FString PropertyName = FunctionName.RightChop(FunctionName.Len() - 3);
					this->PropertyNames->Add(PropertyName);
				}
			}
		}
		return *PropertyNames;
	}
}

