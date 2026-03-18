// Copyright GameDevWare, Denis Zykov 2025

// ReSharper disable CppUseFamiliarTemplateSyntaxForGenericLambdas
#include "FDotNetSurrogateType.h"

#include "GameData/Formulas/FFormulaTypeResolver.h"

FName SelfParameterName = FName(TEXT("Self"));

static bool IsPublic(const UFunction* Function)
{
	return (Function->FunctionFlags & EFunctionFlags::FUNC_Public) != 0;
}
static bool IsPublic(const FProperty* Property)
{
	return (Property->PropertyFlags & EPropertyFlags::CPF_NativeAccessSpecifierPublic) != 0;
}
static bool IsExtensionMethod(const UFunction* Function, const FProperty* TargetType)
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

FDotNetSurrogateType::FDotNetSurrogateType(UClass* SurrogateClass, FProperty* LiteralProperty) :
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
		FString ExtendedTypeText;
		FString Name = this->LiteralField->GetCPPType(&ExtendedTypeText);
		if (ExtendedTypeText.Len() > 0)
		{
			Name.Append(ExtendedTypeText);
		}
		return Name;
	}
	else
	{
		return ClassPtr->GetName();
	}
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

			InitializeStaticFunctions(DeclaringClass, this->StaticFunctions.ToSharedRef());
		}
		FoundFunction = this->StaticFunctions->Find(MemberName);
		return FoundFunction != nullptr;
	}
	else
	{
		if (!this->Functions.IsValid())
		{
			this->Functions = MakeShared<TMap<FString, FFormulaFunction>>();

			InitializeFunctions(DeclaringClass, Functions.ToSharedRef());
		}
		FoundFunction = this->Functions->Find(MemberName);
		return FoundFunction != nullptr;
	}
}

const TArray<FString>& FDotNetSurrogateType::GetFunctionNames(bool bStatic)
{
	UClass* DeclaringClass = Cast<UClass>(this->ClassPtr.Get());
	if (bStatic)
	{
		if (!this->StaticFunctionNames.IsValid())
		{
			this->StaticFunctionNames = MakeShared<TArray<FString>>();
			InitializeStaticFunctionNames(DeclaringClass, this->StaticFunctionNames.ToSharedRef());
		}
		return *this->StaticFunctionNames;
	}
	else
	{
		if (!this->FunctionNames.IsValid())
		{
			this->FunctionNames = MakeShared<TArray<FString>>();
			InitializeFunctionNames(DeclaringClass, this->FunctionNames.ToSharedRef());
		}
		return *this->FunctionNames;
	}
}

bool FDotNetSurrogateType::TryGetProperty(const FString& MemberName, bool bStatic,
                                          const FFormulaProperty*& FoundProperty)
{
	UClass* DeclaringClass = this->ClassPtr.Get();
	if (bStatic)
	{
		if (!this->StaticProperties.IsValid())
		{
			this->StaticProperties = MakeShared<TMap<FString, FFormulaProperty>>();
			InitializeStaticProperties(DeclaringClass, this->StaticProperties.ToSharedRef());
		}
		FoundProperty = this->StaticProperties->Find(MemberName);
		return FoundProperty != nullptr;
	}
	else
	{
		if (!this->Properties.IsValid())
		{
			this->Properties = MakeShared<TMap<FString, FFormulaProperty>>();
			
			InitializeProperties(DeclaringClass, this->Properties.ToSharedRef());
		}
		FoundProperty = this->Properties->Find(MemberName);
		return FoundProperty != nullptr;
	}
}

const TArray<FString>& FDotNetSurrogateType::GetPropertyNames(bool bStatic)
{
	UClass* DeclaringClass = this->ClassPtr.Get();
	if (bStatic)
	{
		if (!this->StaticPropertyNames.IsValid())
		{
			this->StaticPropertyNames = MakeShared<TArray<FString>>();
			InitializeStaticPropertyNames(DeclaringClass, this->StaticPropertyNames.ToSharedRef());
		}
		return *StaticPropertyNames;
	}
	else
	{
		if (!this->PropertyNames.IsValid())
		{
			this->PropertyNames = MakeShared<TArray<FString>>();
			InitializePropertyNames(DeclaringClass, this->PropertyNames.ToSharedRef());
		}
		return *PropertyNames;
	}
}

FFormulaFunction FDotNetSurrogateType::GetRefenceEqualsFunction(UField* DeclaringType)
{
	return FFormulaFunction([](
		const TSharedRef<FFormulaValue>&,
		const FFormulaInvokeArguments& Arguments,
		const UField*,
		const TArray<UField*>*, TSharedPtr<FFormulaValue>& Result)
		{
			const auto Argument1 = Arguments.FindArgument(TEXT("objA"), 0);
			const auto Argument2 = Arguments.FindArgument(TEXT("objB"), 1);
			if (!Argument1 || !Argument2)
			{
				return false;
			}
			const auto LeftOperand = Argument1->UpdatedValue.IsValid()
										 ? Argument1->UpdatedValue
										 : Argument1->Value;
			const auto RightOperand = Argument2->UpdatedValue.IsValid()
										  ? Argument2->UpdatedValue
										  : Argument2->Value;
			UObject* LeftObjectPtr = nullptr;
			UObject* RightObjectPtr = nullptr;
			Result = LeftOperand->TryGetObjectPtr(LeftObjectPtr) &&
					 RightOperand->TryGetObjectPtr(RightObjectPtr) &&
						LeftObjectPtr == RightObjectPtr
						 ? FFormulaValue::TrueBool()
						 : FFormulaValue::FalseBool();
			return true;
		},
		TArray<const FProperty*>(),
		DeclaringType,
		/*bUseClassDefaultObject*/ true);
}

FFormulaFunction FDotNetSurrogateType::GetValueEqualsFunction(UField* DeclaringType)
{
	return FFormulaFunction([](
		const TSharedRef<FFormulaValue>&,
		const FFormulaInvokeArguments& Arguments,
		const UField*,
		const TArray<UField*>*, TSharedPtr<FFormulaValue>& Result)
		{
			const auto LeftArgument = Arguments.FindArgument(TEXT("objA"), 0);
			const auto RightArgument = Arguments.FindArgument(TEXT("objB"), 1);
			if (!LeftArgument || !RightArgument)
			{
				return false;
			}
			const auto LeftOperand = LeftArgument->UpdatedValue.IsValid()
										 ? LeftArgument->UpdatedValue
										 : LeftArgument->Value;
			const auto RightOperand = RightArgument->UpdatedValue.IsValid()
										  ? RightArgument->UpdatedValue
										  : RightArgument->Value;
		
			

			Result = LeftOperand->EqualsTo(RightOperand.ToSharedRef()) ? FFormulaValue::TrueBool() : FFormulaValue::FalseBool();
			return true;
		},
		TArray<const FProperty*>(),
		DeclaringType,
		/*bUseClassDefaultObject*/ true);
}

FFormulaFunction FDotNetSurrogateType::GetToStringFunction(UField* DeclaringType)
{
	return FFormulaFunction([](
		const TSharedRef<FFormulaValue>& Target,
		const FFormulaInvokeArguments&,
		const UField*,
		const TArray<UField*>*, TSharedPtr<FFormulaValue>& Result)
		{
			Result = MakeShared<FFormulaValue>(Target->ToString());
			return true;
		},
		TArray<const FProperty*>(),
		DeclaringType,
		/*bUseClassDefaultObject*/ false);
}

FFormulaFunction FDotNetSurrogateType::GetGetTypeFunction(UField* DeclaringType)
{
	return FFormulaFunction([DeclaringType](
		const TSharedRef<FFormulaValue>& Target,
		const FFormulaInvokeArguments&,
		const UField*,
		const TArray<UField*>*, TSharedPtr<FFormulaValue>& Result)
		{	
			UObject* TargetObjectPtr = nullptr;
			if (Target->TryGetObjectPtr(TargetObjectPtr) && TargetObjectPtr != nullptr)
			{
				Result = MakeShared<FFormulaValue>(static_cast<UObject*>(TargetObjectPtr->GetClass()));
				return true;
			} 
			else if (const FStructProperty* StructProperty = CastField<FStructProperty>(Target->GetType()))
			{
				Result = MakeShared<FFormulaValue>(static_cast<UObject*>(StructProperty->Struct.Get()));
				return true;
			}
			else
			{
				//
				// fallback to `DeclaringType` type because primitives doesn't have USTRUCT/UCLASS instance.
				// we can't return null for x.GetType() and we can't tell called that call `invalid`
				//
				Result = MakeShared<FFormulaValue>(static_cast<UObject*>(DeclaringType));
				return true;
			}
		},
		TArray<const FProperty*>(),
		DeclaringType,
		/*bUseClassDefaultObject*/ false);
}

FFormulaFunction FDotNetSurrogateType::GetEqualsFunction(UField* DeclaringType)
{
	return FFormulaFunction([](
		const TSharedRef<FFormulaValue>& Target,
		const FFormulaInvokeArguments& Arguments,
		const UField*,
		const TArray<UField*>*, TSharedPtr<FFormulaValue>& Result)
		{
			const auto OtherArgument = Arguments.FindArgument(TEXT("other"), 0);
			if (!OtherArgument)
			{
				return false;
			}
			const auto OtherValue = OtherArgument->UpdatedValue.IsValid()
										 ? OtherArgument->UpdatedValue
										 : OtherArgument->Value;

			Result = Target->EqualsTo(OtherValue.ToSharedRef()) ? FFormulaValue::TrueBool() : FFormulaValue::FalseBool();
			return true;
		},
		TArray<const FProperty*>(),
		DeclaringType,
		/*bUseClassDefaultObject*/ false);
}

void FDotNetSurrogateType::InitializeStaticFunctionNames(UClass* DeclaringClass, const TSharedRef<TArray<FString>> NameList)
{
	if (!DeclaringClass) return;
	
	for (TFieldIterator<UFunction> It(DeclaringClass); It; ++It)
	{
		if (const auto Function = *It; IsPublic(Function) && !IsExtensionMethod(Function, this->LiteralField))
		{
			NameList->Add(Function->GetName());
		}
	}
	NameList->Add(TEXT("ReferenceEquals"));
	NameList->Add(TEXT("Equals"));
}
void FDotNetSurrogateType::InitializeStaticFunctions(UClass* DeclaringClass, const TSharedRef<TMap<FString, FFormulaFunction>> FunctionList)
{
	if (!DeclaringClass) return;
	
	for (TFieldIterator<UFunction> It(DeclaringClass); It; ++It)
	{
		if (const auto Function = *It; IsPublic(Function) && !IsExtensionMethod(Function, this->LiteralField))
		{
			FunctionList->Add(
				Function->GetName(),
				FFormulaFunction(
					Function, 
					DeclaringClass, 
					/*bUseClassDefaultObject*/ true));
		}
	}
	FunctionList->Add(TEXT("ReferenceEquals"), GetRefenceEqualsFunction(this->ClassPtr.Get()));
	FunctionList->Add(TEXT("Equals"), GetValueEqualsFunction(this->ClassPtr.Get()));
}
void FDotNetSurrogateType::InitializeFunctionNames(UClass* DeclaringClass, const TSharedRef<TArray<FString>> NameList)
{
	if (!DeclaringClass) return;
	
	for (TFieldIterator<UFunction> It(DeclaringClass); It; ++It)
	{
		if (const auto Function = *It; IsPublic(Function) && IsExtensionMethod(Function, this->LiteralField))
		{
			NameList->Add(Function->GetName());
		}
	}
	NameList->Add(TEXT("ToString"));
	NameList->Add(TEXT("GetType"));
	NameList->Add(TEXT("Equals"));
}
void FDotNetSurrogateType::InitializeFunctions(UClass* DeclaringClass, const TSharedRef<TMap<FString, FFormulaFunction>> FunctionList)
{
	if (!DeclaringClass) return;
	
	for (TFieldIterator<UFunction> It(DeclaringClass); It; ++It)
	{
		if (const auto Function = *It; IsPublic(Function) && IsExtensionMethod(Function, this->LiteralField))
		{
			const auto ExtensionFunctionInvoker = FFormulaFunction::CreateExtensionFunctionInvoker(Function, DeclaringClass);
			auto ExtensionFunctionParameters = FFormulaFunction::GetFunctionParameters(Function);
			ExtensionFunctionParameters.RemoveAt(0);
			
			FunctionList->Add(
				Function->GetName(),
				FFormulaFunction(
					ExtensionFunctionInvoker, 
					ExtensionFunctionParameters,
					DeclaringClass, 
					/*bUseClassDefaultObject*/ false));
		}
	}

	FunctionList->Add(TEXT("ToString"), GetToStringFunction(this->ClassPtr.Get()));
	FunctionList->Add(TEXT("GetType"), GetGetTypeFunction(this->ClassPtr.Get()));
	FunctionList->Add(TEXT("Equals"), GetEqualsFunction(this->ClassPtr.Get()));
}

void FDotNetSurrogateType::InitializeStaticProperties(UClass* DeclaringClass, const TSharedRef<TMap<FString, FFormulaProperty>> PropertyList)
{
	if (!DeclaringClass) return;
	
	for (TFieldIterator<FProperty> It(DeclaringClass); It; ++It)
	{
		if (const auto Property = *It; IsPublic(Property))
		{
			PropertyList->Add(
				Property->GetName(),
				FFormulaProperty(Property, DeclaringClass, /*bUseClassDefaultObject*/ true));
		}
	}
}
void FDotNetSurrogateType::InitializeProperties(UClass* DeclaringClass, const TSharedRef<TMap<FString, FFormulaProperty>> PropertyList)
{
	if (!DeclaringClass) return;
	
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
			for (TFieldIterator<FProperty> It(Setter); It && (It->PropertyFlags & CPF_Parm); ++It)
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
			continue;
			// invalid extension property pair, should be `GetXXX(Self) -> T` and `SetXXX(Self, T) -> void`  
		}

		PropertyList->Add(
			PropertyName,
			FFormulaProperty(
				PropertyType, 
				GetterFunc, 
				SetterFunc, 
				DeclaringClass,
				/*bUseClassDefaultObject*/ false));
	}
}
void FDotNetSurrogateType::InitializeStaticPropertyNames(UClass* DeclaringClass, const TSharedRef<TArray<FString>> NameList)
{
	if (!DeclaringClass) return;

	for (TFieldIterator<FProperty> It(DeclaringClass); It; ++It)
	{
		if (const auto Property = *It; IsPublic(Property))
		{
			NameList->Add(Property->GetName());
		}
	}
}
void FDotNetSurrogateType::InitializePropertyNames(UClass* DeclaringClass, const TSharedRef<TArray<FString>> NameList)
{
	if (!DeclaringClass) return;

	for (TFieldIterator<UFunction> It(DeclaringClass); It; ++It)
	{
		if (const auto Function = *It; IsPublic(Function) && IsExtensionMethod(Function, this->LiteralField))
		{
			FString FunctionName = Function->GetName();
			FString PropertyName = FunctionName.RightChop(3);
			NameList->Add(PropertyName);
		}
	}
}