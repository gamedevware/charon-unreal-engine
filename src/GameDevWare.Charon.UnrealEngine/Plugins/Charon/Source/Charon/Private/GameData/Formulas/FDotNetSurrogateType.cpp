// Copyright GameDevWare, Denis Zykov 2025

#include "FDotNetSurrogateType.h"

FName SelfParameterName = FName(TEXT("Self"));

FDotNetSurrogateType::FDotNetSurrogateType(UClass* SurrogateClass, FProperty* LiteralProperty):
	ClassPtr(SurrogateClass),
	LiteralField(LiteralProperty),
	TypeCode(LiteralProperty ? GetPropertyTypeCode(LiteralProperty) : EFormulaValueType::Struct)
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
	if (this->LiteralField)
	{
		FString* ExtendedTypeText = nullptr;
		FString Name = this->LiteralField->GetCPPType(ExtendedTypeText);
		return ExtendedTypeText ? Name + *ExtendedTypeText : Name;		
	}
	else
	{
		return ClassPtr->GetName();
	}
}

bool IsPublic(const UFunction* Function)
{
	return (Function->FunctionFlags & EFunctionFlags::FUNC_Public) != 0; 
}
bool IsPublic(const FProperty* Property)
{
	return (Property->PropertyFlags & EPropertyFlags::CPF_NativeAccessSpecifierPublic) != 0; 
}
bool IsExtensionMethod(const UFunction* Function, const FProperty* TargetType)
{
	check(Function);
	
	for (TFieldIterator<FProperty> It(Function); It; ++It)
	{
		const FProperty* Argument = *It;
		if ((Argument->PropertyFlags & CPF_ReturnParm) != 0)
		{
			continue;
		}
		
		if (Argument->GetFName() == SelfParameterName && (!TargetType || Argument->SameType(TargetType)))
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
				if (const auto Function = *It; IsPublic(Function) && !IsExtensionMethod(Function, this->LiteralField))
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
				if (const auto Function = *It; IsPublic(Function) && IsExtensionMethod(Function, this->LiteralField))
				{
					const auto ExtensionFunctionInvoker = FFormulaFunction::CreateExtensionFunctionInvoker(Function, DeclaringClass);
					auto ExtensionFunctionParameters = FFormulaFunction::GetFunctionParameters(Function);
					ExtensionFunctionParameters.RemoveAt(0);
					this->Functions->Add(Function->GetName(), FFormulaFunction(ExtensionFunctionInvoker, ExtensionFunctionParameters, DeclaringClass, /*bUseClassDefaultObject*/ false));
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
				if (const auto Function = *It; IsPublic(Function) && !IsExtensionMethod(Function, this->LiteralField))
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
				if (const auto Function = *It; IsPublic(Function) && IsExtensionMethod(Function, this->LiteralField))
				{
					FunctionNames->Add(Function->GetName());
				}
			}
		}
		return *this->FunctionNames;
	}
}

static bool IsExtensionProperty(const UFunction* Function, const FProperty* TargetType)
{
	check(Function);

	if (!IsExtensionMethod(Function, TargetType))
	{
		return false;
	}
	
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
		
		if (Argument->GetFName() == SelfParameterName && (!TargetType || Argument->SameType(TargetType)))
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
				if (const auto Function = *It; IsPublic(Function) && IsExtensionProperty(Function, this->LiteralField))
				{
					FString FunctionName = Function->GetName();
					FString PropertyName = FunctionName.RightChop(3);
					TTuple<UFunction*, UFunction*> GetterAndSetter = ExtensionProperties.FindOrAdd(PropertyName);
					if (FunctionName.StartsWith(TEXT("Get")))
					{
						GetterAndSetter = TTuple<UFunction*, UFunction*>(Function, GetterAndSetter.Value);
					}
					else
					{
						GetterAndSetter = TTuple<UFunction*, UFunction*>(GetterAndSetter.Key, Function);
					}
					ExtensionProperties.Add(PropertyName, GetterAndSetter);
				}
			}

			this->Properties = MakeShared<TMap<FString, FFormulaProperty>>();
			for (const auto& ExtensionProperty : ExtensionProperties)
			{
				FString PropertyName = ExtensionProperty.Key;
				UFunction* Getter = ExtensionProperty.Value.Key;
				UFunction* Setter = ExtensionProperty.Value.Value;
				
				FProperty* PropertyType = nullptr;
				FFormulaPropertyGetterFunc GetterFunc;
				if (Getter)
				{
					const auto GetterInvoker = FFormulaFunction::CreateExtensionFunctionInvoker(Getter, DeclaringClass);
					GetterFunc = FFormulaProperty::CreateGetterFromFunctionInvoker(GetterInvoker);
					PropertyType = Getter->GetReturnProperty();
				}
				FFormulaPropertySetterFunc SetterFunc;
				if (Setter)
				{
					const auto SetterInvoker = FFormulaFunction::CreateExtensionFunctionInvoker(Setter, DeclaringClass);
					SetterFunc = FFormulaProperty::CreateSetterFromFunctionInvoker(SetterInvoker);
					// Setter has 2 parameters -> (Self, Value), it is checked in IsExtensionProperty
					int32 ParameterIndex = 0;
					for( TFieldIterator<FProperty> It(Setter); It && (It->PropertyFlags & CPF_Parm); ++It )
					{
						if (ParameterIndex == 1)
						{
							PropertyType = *It;
						}
						ParameterIndex++;
					}
				}
				
				if (!PropertyType || !Getter || !Setter)
				{
					continue; // invalid extension property pair, should be `GetXXX(Self) -> T` and `SetXXX(Self, T) -> void`  
				}
				
				this->Properties->Add(PropertyName, FFormulaProperty(PropertyType, GetterFunc, SetterFunc, DeclaringClass, /*bUseClassDefaultObject*/ false));
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
				if (const auto Function = *It; IsPublic(Function) && IsExtensionMethod(Function, this->LiteralField))
				{
					FString FunctionName = Function->GetName();
					FString PropertyName = FunctionName.RightChop(3);
					this->PropertyNames->Add(PropertyName);
				}
			}
		}
		return *PropertyNames;
	}
}

