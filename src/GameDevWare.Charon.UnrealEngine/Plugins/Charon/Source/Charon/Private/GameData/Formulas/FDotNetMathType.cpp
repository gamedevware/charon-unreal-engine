// Copyright GameDevWare, Denis Zykov 2025

// ReSharper disable CppUseFamiliarTemplateSyntaxForGenericLambdas
#include "FDotNetMathType.h"
#include "GameData/Formulas/FFormulaProperty.h"
#include "UObject/UnrealType.h"
#include <cmath>

static bool IsNumeric(const EFormulaValueType TypeCode)
{
	switch (TypeCode)
	{
	case EFormulaValueType::Float:
	case EFormulaValueType::Double:
	case EFormulaValueType::Int8:
	case EFormulaValueType::Int16:
	case EFormulaValueType::Int32:
	case EFormulaValueType::Int64:
	case EFormulaValueType::UInt8:
	case EFormulaValueType::UInt16:
	case EFormulaValueType::UInt32:
	case EFormulaValueType::UInt64:
		return true;
	default:
		return false;
	}
}

static bool IsInteger(const EFormulaValueType TypeCode)
{
	switch (TypeCode)
	{
	case EFormulaValueType::Int8:
	case EFormulaValueType::Int16:
	case EFormulaValueType::Int32:
	case EFormulaValueType::Int64:
	case EFormulaValueType::UInt8:
	case EFormulaValueType::UInt16:
	case EFormulaValueType::UInt32:
	case EFormulaValueType::UInt64:
		return true;
	default:
		return false;
	}
}

void FDotNetMathType::InitializeStaticFunctions(UClass* DeclaringClass,
	const TSharedRef<TMap<FString, FFormulaFunction>> FunctionList)
{
	FDotNetSurrogateType::InitializeStaticFunctions(DeclaringClass, FunctionList);

	UField* const DeclaringType = GetTypeClassOrStruct();
	FunctionList->Add(TEXT("Abs"),    GetAbsFunction(DeclaringType));
	FunctionList->Add(TEXT("Clamp"),  GetClampFunction(DeclaringType));
	FunctionList->Add(TEXT("DivRem"), GetDivRemFunction(DeclaringType));
	FunctionList->Add(TEXT("Max"),    GetMaxFunction(DeclaringType));
	FunctionList->Add(TEXT("Min"),    GetMinFunction(DeclaringType));
	FunctionList->Add(TEXT("Round"),  GetRoundFunction(DeclaringType));
	FunctionList->Add(TEXT("Sign"),   GetSignFunction(DeclaringType));
}

void FDotNetMathType::InitializeStaticFunctionNames(UClass* DeclaringClass, const TSharedRef<TArray<FString>> NameList)
{
	FDotNetSurrogateType::InitializeStaticFunctionNames(DeclaringClass, NameList);

	NameList->Add(TEXT("Abs"));
	NameList->Add(TEXT("Clamp"));
	NameList->Add(TEXT("DivRem"));
	NameList->Add(TEXT("Max"));
	NameList->Add(TEXT("Min"));
	NameList->Add(TEXT("Round"));
	NameList->Add(TEXT("Sign"));
}

FFormulaFunction FDotNetMathType::GetAbsFunction(UField* DeclaringType)
{
	return FFormulaFunction([](
		const TSharedRef<FFormulaValue>&,
		const FFormulaInvokeArguments& Arguments,
		const UField*,
		const TArray<UField*>*, TSharedPtr<FFormulaValue>& Result)
		{
			const auto Argument1 = Arguments.FindArgument(TEXT("Value"), 0);
			if (!Argument1)
			{
				return false;
			}
			const auto ValueOperand = Argument1->UpdatedValue.IsValid()
				? Argument1->UpdatedValue
				: Argument1->Value;
			if (!IsNumeric(ValueOperand->GetTypeCode()))
			{
				return false;
			}

			return ValueOperand->VisitValue([&Result](FProperty&, const auto& ValuePtr) -> bool {
				using ValueT = std::decay_t<decltype(ValuePtr)>;
				constexpr bool bIsInteger = std::is_integral_v<ValueT>;
				constexpr bool bIsFloat   = std::is_floating_point_v<ValueT>;
				if constexpr (bIsInteger || bIsFloat)
				{
					if constexpr (std::is_unsigned_v<ValueT>)
					{
						Result = MakeShared<FFormulaValue>(ValuePtr);
					}
					else
					{
						Result = MakeShared<FFormulaValue>(ValuePtr < ValueT{0} ? -ValuePtr : ValuePtr);
					}
					return true;
				}
				else
				{
					return false;
				}
			});
		},
		TArray<const FProperty*>(),
		DeclaringType,
		/*bUseClassDefaultObject*/ true);
}

FFormulaFunction FDotNetMathType::GetClampFunction(UField* DeclaringType)
{
	return FFormulaFunction([](
		const TSharedRef<FFormulaValue>&,
		const FFormulaInvokeArguments& Arguments,
		const UField*,
		const TArray<UField*>*, TSharedPtr<FFormulaValue>& Result)
		{
			const auto Argument1 = Arguments.FindArgument(TEXT("Value"), 0);
			const auto Argument2 = Arguments.FindArgument(TEXT("Min"), 1);
			const auto Argument3 = Arguments.FindArgument(TEXT("Max"), 2);
			if (!Argument1 || !Argument2 || !Argument3)
			{
				return false; // all arguments should be present
			}
			const auto ValueOperand = Argument1->UpdatedValue.IsValid()
				? Argument1->UpdatedValue
				: Argument1->Value;
			const auto MinOperand = Argument2->UpdatedValue.IsValid()
				? Argument2->UpdatedValue
				: Argument2->Value;
			const auto MaxOperand = Argument3->UpdatedValue.IsValid()
				? Argument3->UpdatedValue
				: Argument3->Value;

			if (!IsNumeric(ValueOperand->GetTypeCode()) ||
				!IsNumeric(MinOperand->GetTypeCode())   ||
				!IsNumeric(MaxOperand->GetTypeCode()))
			{
				return false; // all arguments should be numeric type
			}

			return ValueOperand->VisitValue([&MinOperand, &MaxOperand, &ValueOperand, &Result](FProperty&, const auto& ValuePtr) -> bool {
				using ValueT = std::decay_t<decltype(ValuePtr)>;
				constexpr bool bIsInteger = std::is_integral_v<ValueT>;
				constexpr bool bIsFloat   = std::is_floating_point_v<ValueT>;
				if constexpr (bIsInteger || bIsFloat)
				{
					ValueT Value    = ValuePtr;
					ValueT MinValue = {};
					ValueT MaxValue = {};
					MinOperand->TryCopyCompleteValue(MinOperand->GetType(), &MinValue);
					MaxOperand->TryCopyCompleteValue(MaxOperand->GetType(), &MaxValue);
					if (Value < MinValue)
					{
						Result = MakeShared<FFormulaValue>(MinValue);
					}
					else if (Value > MaxValue)
					{
						Result = MakeShared<FFormulaValue>(MaxValue);
					}
					else
					{
						Result = ValueOperand;
					}
					return true;
				}
				else
				{
					return false;
				}
			});
		},
		TArray<const FProperty*>(),
		DeclaringType,
		/*bUseClassDefaultObject*/ true);
}

FFormulaFunction FDotNetMathType::GetDivRemFunction(UField* DeclaringType)
{
	return FFormulaFunction([](
		const TSharedRef<FFormulaValue>&,
		const FFormulaInvokeArguments& Arguments,
		const UField*,
		const TArray<UField*>*, TSharedPtr<FFormulaValue>& Result)
		{
			const auto Argument1 = Arguments.FindArgument(TEXT("A"), 0);
			const auto Argument2 = Arguments.FindArgument(TEXT("B"), 1);
			if (!Argument1 || !Argument2)
			{
				return false;
			}
			const auto DividendOperand = Argument1->UpdatedValue.IsValid()
				? Argument1->UpdatedValue
				: Argument1->Value;
			const auto DivisorOperand = Argument2->UpdatedValue.IsValid()
				? Argument2->UpdatedValue
				: Argument2->Value;

			if (!IsInteger(DividendOperand->GetTypeCode()) ||
				!IsInteger(DivisorOperand->GetTypeCode()))
			{
				return false; // DivRem is defined for integer types only
			}

			return DividendOperand->VisitValue([&DivisorOperand, &Result](FProperty&, const auto& ValuePtr) -> bool {
				using ValueT = std::decay_t<decltype(ValuePtr)>;
				constexpr bool bIsInteger = std::is_integral_v<ValueT>;
				constexpr bool bNotBool   = !std::is_same_v<ValueT, bool>;
				if constexpr (bIsInteger && bNotBool)
				{
					ValueT Divisor = {};
					DivisorOperand->TryCopyCompleteValue(DivisorOperand->GetType(), &Divisor);
					if (Divisor == ValueT{0})
					{
						return false; // division by zero
					}
					Result = MakeShared<FFormulaValue>(static_cast<ValueT>(ValuePtr / Divisor));
					return true;
				}
				else
				{
					return false;
				}
			});
		},
		TArray<const FProperty*>(),
		DeclaringType,
		/*bUseClassDefaultObject*/ true);
}

FFormulaFunction FDotNetMathType::GetMaxFunction(UField* DeclaringType)
{
	return FFormulaFunction([](
		const TSharedRef<FFormulaValue>&,
		const FFormulaInvokeArguments& Arguments,
		const UField*,
		const TArray<UField*>*, TSharedPtr<FFormulaValue>& Result)
		{
			const auto Argument1 = Arguments.FindArgument(TEXT("Val1"), 0);
			const auto Argument2 = Arguments.FindArgument(TEXT("Val2"), 1);
			if (!Argument1 || !Argument2)
			{
				return false;
			}
			const auto Operand1 = Argument1->UpdatedValue.IsValid()
				? Argument1->UpdatedValue
				: Argument1->Value;
			const auto Operand2 = Argument2->UpdatedValue.IsValid()
				? Argument2->UpdatedValue
				: Argument2->Value;

			if (!IsNumeric(Operand1->GetTypeCode()) || !IsNumeric(Operand2->GetTypeCode()))
			{
				return false;
			}

			return Operand1->VisitValue([&Operand2, &Result](FProperty&, const auto& ValuePtr) -> bool {
				using ValueT = std::decay_t<decltype(ValuePtr)>;
				constexpr bool bIsInteger = std::is_integral_v<ValueT>;
				constexpr bool bIsFloat   = std::is_floating_point_v<ValueT>;
				if constexpr (bIsInteger || bIsFloat)
				{
					ValueT Other = {};
					Operand2->TryCopyCompleteValue(Operand2->GetType(), &Other);
					Result = MakeShared<FFormulaValue>(ValuePtr >= Other ? ValuePtr : Other);
					return true;
				}
				else
				{
					return false;
				}
			});
		},
		TArray<const FProperty*>(),
		DeclaringType,
		/*bUseClassDefaultObject*/ true);
}

FFormulaFunction FDotNetMathType::GetMinFunction(UField* DeclaringType)
{
	return FFormulaFunction([](
		const TSharedRef<FFormulaValue>&,
		const FFormulaInvokeArguments& Arguments,
		const UField*,
		const TArray<UField*>*, TSharedPtr<FFormulaValue>& Result)
		{
			const auto Argument1 = Arguments.FindArgument(TEXT("Val1"), 0);
			const auto Argument2 = Arguments.FindArgument(TEXT("Val2"), 1);
			if (!Argument1 || !Argument2)
			{
				return false;
			}
			const auto Operand1 = Argument1->UpdatedValue.IsValid()
				? Argument1->UpdatedValue
				: Argument1->Value;
			const auto Operand2 = Argument2->UpdatedValue.IsValid()
				? Argument2->UpdatedValue
				: Argument2->Value;

			if (!IsNumeric(Operand1->GetTypeCode()) || !IsNumeric(Operand2->GetTypeCode()))
			{
				return false;
			}

			return Operand1->VisitValue([&Operand2, &Result](FProperty&, const auto& ValuePtr) -> bool {
				using ValueT = std::decay_t<decltype(ValuePtr)>;
				constexpr bool bIsInteger = std::is_integral_v<ValueT>;
				constexpr bool bIsFloat   = std::is_floating_point_v<ValueT>;
				if constexpr (bIsInteger || bIsFloat)
				{
					ValueT Other = {};
					Operand2->TryCopyCompleteValue(Operand2->GetType(), &Other);
					Result = MakeShared<FFormulaValue>(ValuePtr <= Other ? ValuePtr : Other);
					return true;
				}
				else
				{
					return false;
				}
			});
		},
		TArray<const FProperty*>(),
		DeclaringType,
		/*bUseClassDefaultObject*/ true);
}

FFormulaFunction FDotNetMathType::GetRoundFunction(UField* DeclaringType)
{
	return FFormulaFunction([](
		const TSharedRef<FFormulaValue>&,
		const FFormulaInvokeArguments& Arguments,
		const UField*,
		const TArray<UField*>*, TSharedPtr<FFormulaValue>& Result)
		{
			const auto Argument1 = Arguments.FindArgument(TEXT("D"), 0);
			if (!Argument1)
			{
				return false;
			}
			const auto ValueOperand = Argument1->UpdatedValue.IsValid()
				? Argument1->UpdatedValue
				: Argument1->Value;
			if (!IsNumeric(ValueOperand->GetTypeCode()))
			{
				return false;
			}

			// Optional second argument: number of decimal digits to round to
			const auto Argument2 = Arguments.FindArgument(TEXT("Digits"), 1);
			int32 Digits = 0;
			if (Argument2)
			{
				const auto DigitsOperand = Argument2->UpdatedValue.IsValid()
					? Argument2->UpdatedValue
					: Argument2->Value;
				DigitsOperand->TryGetInt32(Digits);
			}

			return ValueOperand->VisitValue([Digits, &Result](FProperty&, const auto& ValuePtr) -> bool {
				using ValueT = std::decay_t<decltype(ValuePtr)>;
				if constexpr (std::is_floating_point_v<ValueT>)
				{
					if (Digits == 0)
					{
						Result = MakeShared<FFormulaValue>(static_cast<ValueT>(std::round(static_cast<double>(ValuePtr))));
					}
					else
					{
						const double Scale = std::pow(10.0, static_cast<double>(Digits));
						Result = MakeShared<FFormulaValue>(static_cast<ValueT>(std::round(static_cast<double>(ValuePtr) * Scale) / Scale));
					}
					return true;
				}
				else if constexpr (std::is_integral_v<ValueT>)
				{
					Result = MakeShared<FFormulaValue>(ValuePtr); // integers are already exact
					return true;
				}
				else
				{
					return false;
				}
			});
		},
		TArray<const FProperty*>(),
		DeclaringType,
		/*bUseClassDefaultObject*/ true);
}

FFormulaFunction FDotNetMathType::GetSignFunction(UField* DeclaringType)
{
	return FFormulaFunction([](
		const TSharedRef<FFormulaValue>&,
		const FFormulaInvokeArguments& Arguments,
		const UField*,
		const TArray<UField*>*, TSharedPtr<FFormulaValue>& Result)
		{
			const auto Argument1 = Arguments.FindArgument(TEXT("Value"), 0);
			if (!Argument1)
			{
				return false;
			}
			const auto ValueOperand = Argument1->UpdatedValue.IsValid()
				? Argument1->UpdatedValue
				: Argument1->Value;
			if (!IsNumeric(ValueOperand->GetTypeCode()))
			{
				return false;
			}

			return ValueOperand->VisitValue([&Result](FProperty&, const auto& ValuePtr) -> bool {
				using ValueT = std::decay_t<decltype(ValuePtr)>;
				constexpr bool bIsInteger = std::is_integral_v<ValueT>;
				constexpr bool bIsFloat   = std::is_floating_point_v<ValueT>;
				if constexpr (bIsInteger || bIsFloat)
				{
					int32 Sign;
					if constexpr (std::is_unsigned_v<ValueT>)
					{
						Sign = ValuePtr > ValueT{0} ? 1 : 0;
					}
					else
					{
						Sign = ValuePtr > ValueT{0} ? 1 : (ValuePtr < ValueT{0} ? -1 : 0);
					}
					Result = MakeShared<FFormulaValue>(Sign);
					return true;
				}
				else
				{
					return false;
				}
			});
		},
		TArray<const FProperty*>(),
		DeclaringType,
		/*bUseClassDefaultObject*/ true);
}