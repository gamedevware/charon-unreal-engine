// Copyright GameDevWare, Denis Zykov 2025

// ReSharper disable CppUseFamiliarTemplateSyntaxForGenericLambdas
#include "FDotNetEnumerableType.h"
#include "GameData/Formulas/FFormulaClosure.h"
#include "GameData/Formulas/FFormulaProperty.h"
#include "GameData/Formulas/DotNetTypes/UDotNetBoolean.h"
#include "GameData/Formulas/DotNetTypes/UDotNetDateTime.h"
#include "GameData/Formulas/DotNetTypes/UDotNetDouble.h"
#include "GameData/Formulas/DotNetTypes/UDotNetInt16.h"
#include "GameData/Formulas/DotNetTypes/UDotNetInt32.h"
#include "GameData/Formulas/DotNetTypes/UDotNetInt64.h"
#include "GameData/Formulas/DotNetTypes/UDotNetInt8.h"
#include "GameData/Formulas/DotNetTypes/UDotNetName.h"
#include "GameData/Formulas/DotNetTypes/UDotNetObject.h"
#include "GameData/Formulas/DotNetTypes/UDotNetSingle.h"
#include "GameData/Formulas/DotNetTypes/UDotNetString.h"
#include "GameData/Formulas/DotNetTypes/UDotNetText.h"
#include "GameData/Formulas/DotNetTypes/UDotNetTimeSpan.h"
#include "GameData/Formulas/DotNetTypes/UDotNetUInt16.h"
#include "GameData/Formulas/DotNetTypes/UDotNetUInt32.h"
#include "GameData/Formulas/DotNetTypes/UDotNetUInt64.h"
#include "GameData/Formulas/DotNetTypes/UDotNetUInt8.h"
#include "GameData/Formulas/TFormulaTypeMap.h"
#include "UObject/UnrealType.h"

DEFINE_LOG_CATEGORY_STATIC(LogFormulaEnumerable, Log, All);

namespace FormulaEnumerable
{
	/** Binds the target to its array property and storage. Non-array targets are rejected. */
	static bool TryGetTargetArray(const TSharedRef<FFormulaValue>& Target, const FArrayProperty*& OutArrayProperty, void*& OutArrayPtr)
	{
		OutArrayProperty = CastField<FArrayProperty>(Target->GetType());
		if (!OutArrayProperty)
		{
			return false;
		}

		OutArrayPtr = nullptr;
		return Target->TryGetContainerAddress(OutArrayPtr) && OutArrayPtr != nullptr;
	}

	/** Reads one element as a value. */
	static TSharedRef<FFormulaValue> GetElement(const FArrayProperty* ArrayProperty, FScriptArrayHelper& ArrayHelper, int32 Index)
	{
		return MakeShared<FFormulaValue>(ArrayProperty->Inner, ArrayHelper.GetElementPtr(Index));
	}

	/** A default-constructed value of the given type, returned wherever a sequence has no answer. */
	static TSharedRef<FFormulaValue> MakeDefaultValue(const FProperty* Property)
	{
		return MakeShared<FFormulaValue>(const_cast<FProperty*>(Property));
	}

	/**
	 * Shapes an enumerable method. The body receives the bound array and reports either a
	 * value or a failure recorded on the call arguments.
	 */
	using FEnumerableBody = TFunction<bool(const FArrayProperty*, FScriptArrayHelper&, FFormulaInvokeArguments&, TSharedPtr<FFormulaValue>&)>;

	static FFormulaFunction MakeFunction(UField* DeclaringType, FEnumerableBody Body)
	{
		return FFormulaFunction([Body = MoveTemp(Body)](
			const TSharedRef<FFormulaValue>& Target,
			FFormulaInvokeArguments& Arguments,
			const UField*,
			const TArray<UField*>*,
			TSharedPtr<FFormulaValue>& Result) -> bool
			{
				const FArrayProperty* ArrayProperty = nullptr;
				void* ArrayPtr = nullptr;
				if (!TryGetTargetArray(Target, ArrayProperty, ArrayPtr))
				{
					return false;
				}

				FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayPtr);
				return Body(ArrayProperty, ArrayHelper, Arguments, Result);
			},
			TArray<const FProperty*>(),
			DeclaringType,
			/*bUseClassDefaultObject*/ false);
	}

	/** Reads the value of an argument, preferring an updated out value when present. */
	static TSharedRef<FFormulaValue> GetArgumentValue(const FFormulaInvokeArguments::InvokeArgument& Argument)
	{
		return Argument.UpdatedValue.IsValid() ? Argument.UpdatedValue.ToSharedRef() : Argument.Value;
	}

	/**
	 * Looks up the optional first argument as a lambda. Absent means the caller used the
	 * overload without a predicate or selector; present but not a lambda fails to bind.
	 */
	enum class EClosureArgument { Absent, Found, NotALambda };

	static EClosureArgument FindClosureArgument(const FFormulaInvokeArguments& Arguments, TSharedPtr<FFormulaClosure>& OutClosure)
	{
		const auto* Argument = Arguments.FindArgument(TEXT("predicate"), 0);
		if (!Argument)
		{
			return EClosureArgument::Absent;
		}

		return FFormulaClosure::TryGetFrom(GetArgumentValue(*Argument), OutClosure)
			? EClosureArgument::Found
			: EClosureArgument::NotALambda;
	}

	/** Runs a predicate over one element, recording any evaluation error as the failure reason. */
	static bool TryTestElement(
		const FFormulaClosure& Predicate,
		const TSharedRef<FFormulaValue>& Element,
		FFormulaInvokeArguments& Arguments,
		bool& bOutMatches)
	{
		const TArray<TSharedRef<FFormulaValue>> PredicateArguments { Element };
		const FFormulaExecutionResult PredicateResult = Predicate.Invoke(PredicateArguments);
		if (PredicateResult.HasError())
		{
			UE_LOG(LogFormulaEnumerable, Error,
				TEXT("A lambda predicate failed to evaluate: %s. The element is treated as not matching."),
				*PredicateResult.GetError().Message);
			bOutMatches = false;
			return true;
		}

		if (!PredicateResult.GetValue()->TryGetBoolean(bOutMatches))
		{
			UE_LOG(LogFormulaEnumerable, Error,
				TEXT("A lambda predicate returned '%s' instead of a boolean, so it cannot decide whether an element matches. The element is treated as not matching."),
				*PredicateResult.GetValue()->GetCPPType());
			bOutMatches = false;
			return true;
		}
		return true;
	}

	/**
	 * Collects the indices matching an optional predicate. With no predicate every element
	 * matches, which is what lets one function serve both LINQ overloads.
	 */
	static bool TryGetMatchingIndices(
		const FArrayProperty* ArrayProperty,
		FScriptArrayHelper& ArrayHelper,
		FFormulaInvokeArguments& Arguments,
		TArray<int32>& OutIndices)
	{
		TSharedPtr<FFormulaClosure> Predicate;
		const EClosureArgument PredicateState = FindClosureArgument(Arguments, Predicate);
		if (PredicateState == EClosureArgument::NotALambda)
		{
			return false;
		}

		for (int32 Index = 0; Index < ArrayHelper.Num(); Index++)
		{
			if (PredicateState == EClosureArgument::Absent)
			{
				OutIndices.Add(Index);
				continue;
			}

			bool bMatches = false;
			if (!TryTestElement(*Predicate, GetElement(ArrayProperty, ArrayHelper, Index), Arguments, bMatches))
			{
				return false;
			}
			if (bMatches)
			{
				OutIndices.Add(Index);
			}
		}
		return true;
	}

	/**
	 * Resolves the values an aggregate runs over: the elements themselves, or their
	 * projections when a selector was supplied. This is what makes Min and Min(selector)
	 * one function rather than two.
	 */
	static bool TryGetAggregateValues(
		const FArrayProperty* ArrayProperty,
		FScriptArrayHelper& ArrayHelper,
		FFormulaInvokeArguments& Arguments,
		TArray<TSharedRef<FFormulaValue>>& OutValues)
	{
		TSharedPtr<FFormulaClosure> Selector;
		const EClosureArgument SelectorState = FindClosureArgument(Arguments, Selector);
		if (SelectorState == EClosureArgument::NotALambda)
		{
			return false;
		}

		for (int32 Index = 0; Index < ArrayHelper.Num(); Index++)
		{
			const TSharedRef<FFormulaValue> Element = GetElement(ArrayProperty, ArrayHelper, Index);
			if (SelectorState == EClosureArgument::Absent)
			{
				OutValues.Add(Element);
				continue;
			}

			const TArray<TSharedRef<FFormulaValue>> SelectorArguments { Element };
			const FFormulaExecutionResult Projected = Selector->Invoke(SelectorArguments);
			if (Projected.HasError())
			{
				UE_LOG(LogFormulaEnumerable, Error,
					TEXT("A lambda selector failed to evaluate: %s. The default of '%s' is used for this element."),
					*Projected.GetError().Message, *FFormulaValue::GetExtendedCppName(ArrayProperty->Inner));
				OutValues.Add(MakeDefaultValue(ArrayProperty->Inner));
				continue;
			}
			OutValues.Add(Projected.GetValue());
		}
		return true;
	}

	/**
	 * Builds a numeric result in the sequence's own element type, the way .NET Sum
	 * returns int for a sequence of int rather than widening to long or double.
	 */
	static bool TryMakeNumericResult(const FProperty* ElementProperty, double Total, TSharedPtr<FFormulaValue>& Result)
	{
		switch (GetFormulaValueTypeCode(ElementProperty))
		{
		case EFormulaValueType::UInt8:  Result = MakeShared<FFormulaValue>(static_cast<uint8>(Total)); return true;
		case EFormulaValueType::UInt16: Result = MakeShared<FFormulaValue>(static_cast<uint16>(Total)); return true;
		case EFormulaValueType::UInt32: Result = MakeShared<FFormulaValue>(static_cast<uint32>(Total)); return true;
		case EFormulaValueType::UInt64: Result = MakeShared<FFormulaValue>(static_cast<uint64>(Total)); return true;
		case EFormulaValueType::Int8:   Result = MakeShared<FFormulaValue>(static_cast<int8>(Total)); return true;
		case EFormulaValueType::Int16:  Result = MakeShared<FFormulaValue>(static_cast<int16>(Total)); return true;
		case EFormulaValueType::Int32:  Result = MakeShared<FFormulaValue>(static_cast<int32>(Total)); return true;
		case EFormulaValueType::Int64:  Result = MakeShared<FFormulaValue>(static_cast<int64>(Total)); return true;
		case EFormulaValueType::Float:  Result = MakeShared<FFormulaValue>(static_cast<float>(Total)); return true;
		case EFormulaValueType::Double: Result = MakeShared<FFormulaValue>(Total); return true;
		default: return false; // non-numeric sequences have no sum or average
		}
	}

	/** Adds one value into a running total. Fails for non-numeric values. */
	static bool TryAddToTotal(const TSharedRef<FFormulaValue>& Value, double& Total)
	{
		int64 IntegralValue = 0;
		if (Value->TryGetInt64(IntegralValue))
		{
			Total += static_cast<double>(IntegralValue);
			return true;
		}

		return Value->VisitValue([&Total](const FProperty&, const auto& InValue) -> bool
		{
			using ValueT = std::decay_t<decltype(InValue)>;
			if constexpr (std::is_floating_point_v<ValueT>)
			{
				Total += static_cast<double>(InValue);
				return true;
			}
			else
			{
				return false;
			}
		});
	}

	/** Folds the values with TryCompare, used by both Min and Max. */
	static bool TryGetExtreme(
		const TArray<TSharedRef<FFormulaValue>>& Values,
		int32 WantedSign,
		const FProperty* ElementProperty,
		const TCHAR* OperationName,
		TSharedPtr<FFormulaValue>& Result)
	{
		if (Values.Num() == 0)
		{
			UE_LOG(LogFormulaEnumerable, Error,
				TEXT("'%s' has no value for an empty sequence, so the default of '%s' is returned."),
				OperationName, *FFormulaValue::GetExtendedCppName(ElementProperty));
			Result = MakeDefaultValue(ElementProperty);
			return true;
		}

		TSharedRef<FFormulaValue> Extreme = Values[0];
		for (int32 Index = 1; Index < Values.Num(); Index++)
		{
			int32 Sign = 0;
			if (!Values[Index]->TryCompare(Extreme, Sign))
			{
				UE_LOG(LogFormulaEnumerable, Error,
					TEXT("'%s' cannot order '%s' against '%s', so this element is skipped."),
					OperationName, *Values[Index]->GetCPPType(), *Extreme->GetCPPType());
				continue;
			}
			if (Sign == WantedSign)
			{
				Extreme = Values[Index];
			}
		}

		Result = Extreme;
		return true;
	}

	/**
	 * Builds the result sequence by copying the chosen elements into a new array of the
	 * *source* array property.
	 *
	 * Reusing the source property is what lets element-preserving operators work for struct
	 * and enum elements: FNewExpression::TryCreateArray cannot synthesize an FArrayProperty
	 * for those, but here none has to be synthesized at all.
	 */
	static bool TryMakeArrayResult(
		const FArrayProperty* ArrayProperty,
		FScriptArrayHelper& SourceHelper,
		const TArray<int32>& Indices,
		TSharedPtr<FFormulaValue>& Result)
	{
		const TSharedRef<FFormulaValue> NewArray = MakeShared<FFormulaValue>(static_cast<FProperty*>(const_cast<FArrayProperty*>(ArrayProperty)));

		void* NewArrayPtr = nullptr;
		if (!NewArray->TryGetContainerAddress(NewArrayPtr) || !NewArrayPtr)
		{
			return false;
		}

		FScriptArrayHelper DestinationHelper(ArrayProperty, NewArrayPtr);
		for (const int32 Index : Indices)
		{
			const int32 AddedIndex = DestinationHelper.AddValue();
			ArrayProperty->Inner->CopyCompleteValue(DestinationHelper.GetElementPtr(AddedIndex), SourceHelper.GetElementPtr(Index));
		}

		Result = NewArray;
		return true;
	}

	/** Orders element indices by a key selector, stably, using TryCompare. */
	static bool TryOrderIndices(
		const FArrayProperty* ArrayProperty,
		FScriptArrayHelper& ArrayHelper,
		FFormulaInvokeArguments& Arguments,
		bool bDescending,
		TArray<int32>& OutIndices)
	{
		TArray<TSharedRef<FFormulaValue>> Keys;
		if (!TryGetAggregateValues(ArrayProperty, ArrayHelper, Arguments, Keys))
		{
			return false;
		}

		OutIndices.Reserve(Keys.Num());
		for (int32 Index = 0; Index < Keys.Num(); Index++)
		{
			OutIndices.Add(Index);
		}

		bool bAllComparable = true;
		OutIndices.StableSort([&Keys, bDescending, &bAllComparable](int32 Left, int32 Right)
		{
			int32 Sign = 0;
			if (!Keys[Left]->TryCompare(Keys[Right], Sign))
			{
				bAllComparable = false;
				return false;
			}
			return bDescending ? Sign > 0 : Sign < 0;
		});

		return bAllComparable;
	}

	/** Collects the leading run of elements satisfying a predicate, or everything after it. */
	static bool TryGetWhileIndices(
		const FArrayProperty* ArrayProperty,
		FScriptArrayHelper& ArrayHelper,
		FFormulaInvokeArguments& Arguments,
		bool bTake,
		TArray<int32>& OutIndices)
	{
		TSharedPtr<FFormulaClosure> Predicate;
		if (FindClosureArgument(Arguments, Predicate) != EClosureArgument::Found)
		{
			return false;
		}

		int32 Index = 0;
		for (; Index < ArrayHelper.Num(); Index++)
		{
			bool bMatches = false;
			if (!TryTestElement(*Predicate, GetElement(ArrayProperty, ArrayHelper, Index), Arguments, bMatches))
			{
				return false;
			}
			if (!bMatches)
			{
				break;
			}
			if (bTake)
			{
				OutIndices.Add(Index);
			}
		}

		if (!bTake)
		{
			for (; Index < ArrayHelper.Num(); Index++)
			{
				OutIndices.Add(Index);
			}
		}
		return true;
	}

	/**
	 * Finds a declared array property for a projected element type.
	 *
	 * Select is the one operator whose result element type differs from the source, so it
	 * cannot reuse the source array property. Struct and enum projections have no declared
	 * array property and are a documented gap.
	 */
	static FArrayProperty* FindArrayPropertyForTypeCode(EFormulaValueType TypeCode)
	{
		switch (TypeCode)
		{
		case EFormulaValueType::Boolean:  return UDotNetBoolean::GetArrayProperty();
		case EFormulaValueType::UInt8:    return UDotNetUInt8::GetArrayProperty();
		case EFormulaValueType::UInt16:   return UDotNetUInt16::GetArrayProperty();
		case EFormulaValueType::UInt32:   return UDotNetUInt32::GetArrayProperty();
		case EFormulaValueType::UInt64:   return UDotNetUInt64::GetArrayProperty();
		case EFormulaValueType::Int8:     return UDotNetInt8::GetArrayProperty();
		case EFormulaValueType::Int16:    return UDotNetInt16::GetArrayProperty();
		case EFormulaValueType::Int32:    return UDotNetInt32::GetArrayProperty();
		case EFormulaValueType::Int64:    return UDotNetInt64::GetArrayProperty();
		case EFormulaValueType::Float:    return UDotNetSingle::GetArrayProperty();
		case EFormulaValueType::Double:   return UDotNetDouble::GetArrayProperty();
		case EFormulaValueType::Timespan: return UDotNetTimeSpan::GetArrayProperty();
		case EFormulaValueType::DateTime: return UDotNetDateTime::GetArrayProperty();
		case EFormulaValueType::String:   return UDotNetString::GetArrayProperty();
		case EFormulaValueType::Name:     return UDotNetName::GetArrayProperty();
		case EFormulaValueType::Text:     return UDotNetText::GetArrayProperty();
		case EFormulaValueType::Null:
		case EFormulaValueType::ObjectPtr: return UDotNetObject::GetArrayProperty();
		default: return nullptr; // Struct and Enum have no declared array property
		}
	}

	/** Reads the second sequence argument of a set operation as a list of values. */
	static bool TryGetSecondSequence(const FFormulaInvokeArguments& Arguments, TArray<TSharedRef<FFormulaValue>>& OutValues)
	{
		const auto* SecondArgument = Arguments.FindArgument(TEXT("second"), 0);
		if (!SecondArgument)
		{
			return false;
		}

		const TSharedRef<FFormulaValue> SecondValue = GetArgumentValue(*SecondArgument);
		const FArrayProperty* SecondArrayProperty = nullptr;
		void* SecondArrayPtr = nullptr;
		if (!TryGetTargetArray(SecondValue, SecondArrayProperty, SecondArrayPtr))
		{
			return false;
		}

		FScriptArrayHelper SecondHelper(SecondArrayProperty, SecondArrayPtr);
		for (int32 Index = 0; Index < SecondHelper.Num(); Index++)
		{
			OutValues.Add(GetElement(SecondArrayProperty, SecondHelper, Index));
		}
		return true;
	}

	/** True when Values already holds something equal to Candidate. */
	static bool ContainsValue(const TArray<TSharedRef<FFormulaValue>>& Values, const TSharedRef<FFormulaValue>& Candidate)
	{
		for (const auto& Value : Values)
		{
			if (Value->EqualsTo(Candidate))
			{
				return true;
			}
		}
		return false;
	}

	/**
	 * Builds the result from values rather than source indices, for the operations whose
	 * output includes elements of a second sequence.
	 */
	static bool TryMakeArrayResultFromValues(
		const FArrayProperty* ArrayProperty,
		const TArray<TSharedRef<FFormulaValue>>& Values,
		FFormulaInvokeArguments& Arguments,
		TSharedPtr<FFormulaValue>& Result)
	{
		const TSharedRef<FFormulaValue> NewArray = MakeShared<FFormulaValue>(static_cast<FProperty*>(const_cast<FArrayProperty*>(ArrayProperty)));

		void* NewArrayPtr = nullptr;
		if (!NewArray->TryGetContainerAddress(NewArrayPtr) || !NewArrayPtr)
		{
			return false;
		}

		FScriptArrayHelper DestinationHelper(ArrayProperty, NewArrayPtr);
		for (const auto& Value : Values)
		{
			const int32 AddedIndex = DestinationHelper.AddValue();
			if (!Value->TryCopyCompleteValue(ArrayProperty->Inner, DestinationHelper.GetElementPtr(AddedIndex)))
			{
				UE_LOG(LogFormulaEnumerable, Error,
					TEXT("Could not store a '%s' into '%s', so the default is used for this element."),
					*Value->GetCPPType(), *FFormulaValue::GetExtendedCppName(ArrayProperty));
				ArrayProperty->Inner->ClearValue(DestinationHelper.GetElementPtr(AddedIndex));
			}
		}

		Result = NewArray;
		return true;
	}

	/** Reads a required integer argument, used by Skip and Take. */
	static bool TryGetCountArgument(const FFormulaInvokeArguments& Arguments, int32& OutCount)
	{
		const auto* CountArgument = Arguments.FindArgument(TEXT("count"), 0);
		return CountArgument && GetArgumentValue(*CountArgument)->TryGetInt32(OutCount);
	}
}

void FDotNetEnumerableType::InitializeFunctionNames(UClass* DeclaringClass, const TSharedRef<TArray<FString>> NameList)
{
	FDotNetSurrogateType::InitializeFunctionNames(DeclaringClass, NameList);

	NameList->Add(TEXT("Count"));
	NameList->Add(TEXT("Any"));
	NameList->Add(TEXT("All"));
	NameList->Add(TEXT("First"));
	NameList->Add(TEXT("FirstOrDefault"));
	NameList->Add(TEXT("Last"));
	NameList->Add(TEXT("LastOrDefault"));
	NameList->Add(TEXT("Single"));
	NameList->Add(TEXT("SingleOrDefault"));
	NameList->Add(TEXT("ElementAt"));
	NameList->Add(TEXT("ElementAtOrDefault"));
	NameList->Add(TEXT("Contains"));
	NameList->Add(TEXT("Min"));
	NameList->Add(TEXT("Max"));
	NameList->Add(TEXT("Sum"));
	NameList->Add(TEXT("Average"));

	NameList->Add(TEXT("ToArray"));
	NameList->Add(TEXT("ToList"));
	NameList->Add(TEXT("Where"));
	NameList->Add(TEXT("Skip"));
	NameList->Add(TEXT("Take"));
	NameList->Add(TEXT("SkipWhile"));
	NameList->Add(TEXT("TakeWhile"));
	NameList->Add(TEXT("Distinct"));
	NameList->Add(TEXT("Reverse"));
	NameList->Add(TEXT("OrderBy"));
	NameList->Add(TEXT("OrderByDescending"));
	NameList->Add(TEXT("Select"));
	NameList->Add(TEXT("Concat"));
	NameList->Add(TEXT("Union"));
	NameList->Add(TEXT("Except"));
	NameList->Add(TEXT("Intersect"));
}

void FDotNetEnumerableType::InitializeFunctions(UClass* DeclaringClass, const TSharedRef<TMap<FString, FFormulaFunction>> FunctionList)
{
	FDotNetSurrogateType::InitializeFunctions(DeclaringClass, FunctionList);

	using namespace FormulaEnumerable;
	UField* const DeclaringType = GetTypeClassOrStruct();

	FunctionList->Add(TEXT("Count"), MakeFunction(DeclaringType,
		[](const FArrayProperty* ArrayProperty, FScriptArrayHelper& ArrayHelper, FFormulaInvokeArguments& Arguments, TSharedPtr<FFormulaValue>& Result) -> bool
		{
			TArray<int32> Matching;
			if (!TryGetMatchingIndices(ArrayProperty, ArrayHelper, Arguments, Matching))
			{
				return false;
			}
			Result = MakeShared<FFormulaValue>(Matching.Num());
			return true;
		}));

	FunctionList->Add(TEXT("Any"), MakeFunction(DeclaringType,
		[](const FArrayProperty* ArrayProperty, FScriptArrayHelper& ArrayHelper, FFormulaInvokeArguments& Arguments, TSharedPtr<FFormulaValue>& Result) -> bool
		{
			TArray<int32> Matching;
			if (!TryGetMatchingIndices(ArrayProperty, ArrayHelper, Arguments, Matching))
			{
				return false;
			}
			Result = Matching.Num() > 0 ? FFormulaValue::TrueBool() : FFormulaValue::FalseBool();
			return true;
		}));

	FunctionList->Add(TEXT("All"), MakeFunction(DeclaringType,
		[](const FArrayProperty* ArrayProperty, FScriptArrayHelper& ArrayHelper, FFormulaInvokeArguments& Arguments, TSharedPtr<FFormulaValue>& Result) -> bool
		{
			// All has no predicate-less overload in .NET
			TSharedPtr<FFormulaClosure> Predicate;
			if (FindClosureArgument(Arguments, Predicate) != EClosureArgument::Found)
			{
				return false;
			}

			TArray<int32> Matching;
			if (!TryGetMatchingIndices(ArrayProperty, ArrayHelper, Arguments, Matching))
			{
				return false;
			}
			Result = Matching.Num() == ArrayHelper.Num() ? FFormulaValue::TrueBool() : FFormulaValue::FalseBool();
			return true;
		}));

	const auto MakePickFunction = [DeclaringType](bool bFromEnd, bool bOrDefault)
	{
		return MakeFunction(DeclaringType,
			[bFromEnd, bOrDefault](const FArrayProperty* ArrayProperty, FScriptArrayHelper& ArrayHelper, FFormulaInvokeArguments& Arguments, TSharedPtr<FFormulaValue>& Result) -> bool
			{
				TArray<int32> Matching;
				if (!TryGetMatchingIndices(ArrayProperty, ArrayHelper, Arguments, Matching))
				{
					return false;
				}

				if (Matching.Num() == 0)
				{
					if (!bOrDefault)
					{
						UE_LOG(LogFormulaEnumerable, Error,
							TEXT("'%s' found no element, so the default of '%s' is returned."),
							bFromEnd ? TEXT("Last") : TEXT("First"), *FFormulaValue::GetExtendedCppName(ArrayProperty->Inner));
					}
					Result = MakeDefaultValue(ArrayProperty->Inner);
					return true;
				}

				Result = GetElement(ArrayProperty, ArrayHelper, bFromEnd ? Matching.Last() : Matching[0]);
				return true;
			});
	};

	FunctionList->Add(TEXT("First"), MakePickFunction(/*bFromEnd*/ false, /*bOrDefault*/ false));
	FunctionList->Add(TEXT("FirstOrDefault"), MakePickFunction(/*bFromEnd*/ false, /*bOrDefault*/ true));
	FunctionList->Add(TEXT("Last"), MakePickFunction(/*bFromEnd*/ true, /*bOrDefault*/ false));
	FunctionList->Add(TEXT("LastOrDefault"), MakePickFunction(/*bFromEnd*/ true, /*bOrDefault*/ true));

	const auto MakeSingleFunction = [DeclaringType](bool bOrDefault)
	{
		return MakeFunction(DeclaringType,
			[bOrDefault](const FArrayProperty* ArrayProperty, FScriptArrayHelper& ArrayHelper, FFormulaInvokeArguments& Arguments, TSharedPtr<FFormulaValue>& Result) -> bool
			{
				TArray<int32> Matching;
				if (!TryGetMatchingIndices(ArrayProperty, ArrayHelper, Arguments, Matching))
				{
					return false;
				}

				if (Matching.Num() > 1)
				{
					UE_LOG(LogFormulaEnumerable, Error,
						TEXT("'Single' expected one element but found %d. The first match is returned."),
						Matching.Num());
				}

				if (Matching.Num() == 0)
				{
					if (!bOrDefault)
					{
						UE_LOG(LogFormulaEnumerable, Error,
							TEXT("'Single' found no element, so the default of '%s' is returned."),
							*FFormulaValue::GetExtendedCppName(ArrayProperty->Inner));
					}
					Result = MakeDefaultValue(ArrayProperty->Inner);
					return true;
				}

				Result = GetElement(ArrayProperty, ArrayHelper, Matching[0]);
				return true;
			});
	};

	FunctionList->Add(TEXT("Single"), MakeSingleFunction(/*bOrDefault*/ false));
	FunctionList->Add(TEXT("SingleOrDefault"), MakeSingleFunction(/*bOrDefault*/ true));

	const auto MakeElementAtFunction = [DeclaringType](bool bOrDefault)
	{
		return MakeFunction(DeclaringType,
			[bOrDefault](const FArrayProperty* ArrayProperty, FScriptArrayHelper& ArrayHelper, FFormulaInvokeArguments& Arguments, TSharedPtr<FFormulaValue>& Result) -> bool
			{
				const auto* IndexArgument = Arguments.FindArgument(TEXT("index"), 0);
				int32 Index = 0;
				if (!IndexArgument || !GetArgumentValue(*IndexArgument)->TryGetInt32(Index))
				{
					return false;
				}

				if (Index < 0 || Index >= ArrayHelper.Num())
				{
					if (!bOrDefault)
					{
						UE_LOG(LogFormulaEnumerable, Error,
							TEXT("'ElementAt' index %d is outside a sequence of %d, so the default of '%s' is returned."),
							Index, ArrayHelper.Num(), *FFormulaValue::GetExtendedCppName(ArrayProperty->Inner));
					}
					Result = MakeDefaultValue(ArrayProperty->Inner);
					return true;
				}

				Result = GetElement(ArrayProperty, ArrayHelper, Index);
				return true;
			});
	};

	FunctionList->Add(TEXT("ElementAt"), MakeElementAtFunction(/*bOrDefault*/ false));
	FunctionList->Add(TEXT("ElementAtOrDefault"), MakeElementAtFunction(/*bOrDefault*/ true));

	FunctionList->Add(TEXT("Contains"), MakeFunction(DeclaringType,
		[](const FArrayProperty* ArrayProperty, FScriptArrayHelper& ArrayHelper, FFormulaInvokeArguments& Arguments, TSharedPtr<FFormulaValue>& Result) -> bool
		{
			const auto* ValueArgument = Arguments.FindArgument(TEXT("value"), 0);
			if (!ValueArgument)
			{
				return false;
			}
			const TSharedRef<FFormulaValue> Wanted = GetArgumentValue(*ValueArgument);

			for (int32 Index = 0; Index < ArrayHelper.Num(); Index++)
			{
				if (GetElement(ArrayProperty, ArrayHelper, Index)->EqualsTo(Wanted))
				{
					Result = FFormulaValue::TrueBool();
					return true;
				}
			}
			Result = FFormulaValue::FalseBool();
			return true;
		}));

	FunctionList->Add(TEXT("Min"), MakeFunction(DeclaringType,
		[](const FArrayProperty* ArrayProperty, FScriptArrayHelper& ArrayHelper, FFormulaInvokeArguments& Arguments, TSharedPtr<FFormulaValue>& Result) -> bool
		{
			TArray<TSharedRef<FFormulaValue>> Values;
			return TryGetAggregateValues(ArrayProperty, ArrayHelper, Arguments, Values) &&
				TryGetExtreme(Values, /*WantedSign*/ -1, ArrayProperty->Inner, TEXT("Min"), Result);
		}));

	FunctionList->Add(TEXT("Max"), MakeFunction(DeclaringType,
		[](const FArrayProperty* ArrayProperty, FScriptArrayHelper& ArrayHelper, FFormulaInvokeArguments& Arguments, TSharedPtr<FFormulaValue>& Result) -> bool
		{
			TArray<TSharedRef<FFormulaValue>> Values;
			return TryGetAggregateValues(ArrayProperty, ArrayHelper, Arguments, Values) &&
				TryGetExtreme(Values, /*WantedSign*/ 1, ArrayProperty->Inner, TEXT("Max"), Result);
		}));

	FunctionList->Add(TEXT("Sum"), MakeFunction(DeclaringType,
		[](const FArrayProperty* ArrayProperty, FScriptArrayHelper& ArrayHelper, FFormulaInvokeArguments& Arguments, TSharedPtr<FFormulaValue>& Result) -> bool
		{
			TArray<TSharedRef<FFormulaValue>> Values;
			if (!TryGetAggregateValues(ArrayProperty, ArrayHelper, Arguments, Values))
			{
				return false;
			}

			// .NET sums an empty sequence to zero rather than failing
			double Total = 0.0;
			for (const auto& Value : Values)
			{
				if (!TryAddToTotal(Value, Total))
				{
					return false;
				}
			}

			// the sum keeps the summed type, so a sequence of int sums to an int
			const FProperty* SumType = Values.Num() > 0 ? Values[0]->GetType() : ArrayProperty->Inner;
			return TryMakeNumericResult(SumType, Total, Result);
		}));

	FunctionList->Add(TEXT("Average"), MakeFunction(DeclaringType,
		[](const FArrayProperty* ArrayProperty, FScriptArrayHelper& ArrayHelper, FFormulaInvokeArguments& Arguments, TSharedPtr<FFormulaValue>& Result) -> bool
		{
			TArray<TSharedRef<FFormulaValue>> Values;
			if (!TryGetAggregateValues(ArrayProperty, ArrayHelper, Arguments, Values))
			{
				return false;
			}

			if (Values.Num() == 0)
			{
				UE_LOG(LogFormulaEnumerable, Error,
					TEXT("'Average' has no value for an empty sequence, so zero is returned."));
				Result = MakeShared<FFormulaValue>(0.0);
				return true;
			}

			double Total = 0.0;
			for (const auto& Value : Values)
			{
				if (!TryAddToTotal(Value, Total))
				{
					return false;
				}
			}

			// unlike Sum, .NET Average widens an integer sequence to double
			const double Mean = Total / static_cast<double>(Values.Num());
			if (Values[0]->GetTypeCode() == EFormulaValueType::Float)
			{
				Result = MakeShared<FFormulaValue>(static_cast<float>(Mean));
			}
			else
			{
				Result = MakeShared<FFormulaValue>(Mean);
			}
			return true;
		}));

	// Sequence results. Every operator here preserves the element type, so each one copies
	// into a new array of the source array property rather than synthesizing one.

	const auto MakeCopyAllFunction = [DeclaringType]()
	{
		return MakeFunction(DeclaringType,
			[](const FArrayProperty* ArrayProperty, FScriptArrayHelper& ArrayHelper, FFormulaInvokeArguments&, TSharedPtr<FFormulaValue>& Result) -> bool
			{
				TArray<int32> Indices;
				for (int32 Index = 0; Index < ArrayHelper.Num(); Index++)
				{
					Indices.Add(Index);
				}
				return TryMakeArrayResult(ArrayProperty, ArrayHelper, Indices, Result);
			});
	};

	FunctionList->Add(TEXT("ToArray"), MakeCopyAllFunction());
	FunctionList->Add(TEXT("ToList"), MakeCopyAllFunction());

	FunctionList->Add(TEXT("Where"), MakeFunction(DeclaringType,
		[](const FArrayProperty* ArrayProperty, FScriptArrayHelper& ArrayHelper, FFormulaInvokeArguments& Arguments, TSharedPtr<FFormulaValue>& Result) -> bool
		{
			TSharedPtr<FFormulaClosure> Predicate;
			if (FindClosureArgument(Arguments, Predicate) != EClosureArgument::Found)
			{
				return false;
			}

			TArray<int32> Matching;
			return TryGetMatchingIndices(ArrayProperty, ArrayHelper, Arguments, Matching) &&
				TryMakeArrayResult(ArrayProperty, ArrayHelper, Matching, Result);
		}));

	const auto MakeSkipTakeFunction = [DeclaringType](bool bTake)
	{
		return MakeFunction(DeclaringType,
			[bTake](const FArrayProperty* ArrayProperty, FScriptArrayHelper& ArrayHelper, FFormulaInvokeArguments& Arguments, TSharedPtr<FFormulaValue>& Result) -> bool
			{
				int32 RequestedCount = 0;
				if (!TryGetCountArgument(Arguments, RequestedCount))
				{
					return false;
				}

				// .NET clamps rather than failing on counts outside the sequence
				const int32 Clamped = FMath::Clamp(RequestedCount, 0, ArrayHelper.Num());
				TArray<int32> Indices;
				const int32 Start = bTake ? 0 : Clamped;
				const int32 End = bTake ? Clamped : ArrayHelper.Num();
				for (int32 Index = Start; Index < End; Index++)
				{
					Indices.Add(Index);
				}
				return TryMakeArrayResult(ArrayProperty, ArrayHelper, Indices, Result);
			});
	};

	FunctionList->Add(TEXT("Skip"), MakeSkipTakeFunction(/*bTake*/ false));
	FunctionList->Add(TEXT("Take"), MakeSkipTakeFunction(/*bTake*/ true));

	const auto MakeWhileFunction = [DeclaringType](bool bTake)
	{
		return MakeFunction(DeclaringType,
			[bTake](const FArrayProperty* ArrayProperty, FScriptArrayHelper& ArrayHelper, FFormulaInvokeArguments& Arguments, TSharedPtr<FFormulaValue>& Result) -> bool
			{
				TArray<int32> Indices;
				return TryGetWhileIndices(ArrayProperty, ArrayHelper, Arguments, bTake, Indices) &&
					TryMakeArrayResult(ArrayProperty, ArrayHelper, Indices, Result);
			});
	};

	FunctionList->Add(TEXT("SkipWhile"), MakeWhileFunction(/*bTake*/ false));
	FunctionList->Add(TEXT("TakeWhile"), MakeWhileFunction(/*bTake*/ true));

	FunctionList->Add(TEXT("Distinct"), MakeFunction(DeclaringType,
		[](const FArrayProperty* ArrayProperty, FScriptArrayHelper& ArrayHelper, FFormulaInvokeArguments&, TSharedPtr<FFormulaValue>& Result) -> bool
		{
			TArray<int32> Indices;
			TArray<TSharedRef<FFormulaValue>> Seen;
			for (int32 Index = 0; Index < ArrayHelper.Num(); Index++)
			{
				const TSharedRef<FFormulaValue> Element = GetElement(ArrayProperty, ArrayHelper, Index);
				bool bAlreadySeen = false;
				for (const auto& SeenValue : Seen)
				{
					if (SeenValue->EqualsTo(Element))
					{
						bAlreadySeen = true;
						break;
					}
				}
				if (!bAlreadySeen)
				{
					Seen.Add(Element);
					Indices.Add(Index);
				}
			}
			return TryMakeArrayResult(ArrayProperty, ArrayHelper, Indices, Result);
		}));

	FunctionList->Add(TEXT("Reverse"), MakeFunction(DeclaringType,
		[](const FArrayProperty* ArrayProperty, FScriptArrayHelper& ArrayHelper, FFormulaInvokeArguments&, TSharedPtr<FFormulaValue>& Result) -> bool
		{
			TArray<int32> Indices;
			for (int32 Index = ArrayHelper.Num() - 1; Index >= 0; Index--)
			{
				Indices.Add(Index);
			}
			return TryMakeArrayResult(ArrayProperty, ArrayHelper, Indices, Result);
		}));

	const auto MakeOrderByFunction = [DeclaringType](bool bDescending)
	{
		return MakeFunction(DeclaringType,
			[bDescending](const FArrayProperty* ArrayProperty, FScriptArrayHelper& ArrayHelper, FFormulaInvokeArguments& Arguments, TSharedPtr<FFormulaValue>& Result) -> bool
			{
				TArray<int32> Ordered;
				return TryOrderIndices(ArrayProperty, ArrayHelper, Arguments, bDescending, Ordered) &&
					TryMakeArrayResult(ArrayProperty, ArrayHelper, Ordered, Result);
			});
	};

	FunctionList->Add(TEXT("OrderBy"), MakeOrderByFunction(/*bDescending*/ false));
	FunctionList->Add(TEXT("OrderByDescending"), MakeOrderByFunction(/*bDescending*/ true));

	// Set operations. Each takes a second sequence and keeps the source element type.

	enum class ESetOperation { Concat, Union, Except, Intersect };

	const auto MakeSetFunction = [DeclaringType](ESetOperation Operation)
	{
		return MakeFunction(DeclaringType,
			[Operation](const FArrayProperty* ArrayProperty, FScriptArrayHelper& ArrayHelper, FFormulaInvokeArguments& Arguments, TSharedPtr<FFormulaValue>& Result) -> bool
			{
				TArray<TSharedRef<FFormulaValue>> Second;
				if (!TryGetSecondSequence(Arguments, Second))
				{
					return false;
				}

				TArray<TSharedRef<FFormulaValue>> Output;
				for (int32 Index = 0; Index < ArrayHelper.Num(); Index++)
				{
					const TSharedRef<FFormulaValue> Element = GetElement(ArrayProperty, ArrayHelper, Index);
					switch (Operation)
					{
					case ESetOperation::Concat:
						Output.Add(Element);
						break;
					case ESetOperation::Union:
						// Union yields distinct elements of both sequences
						if (!ContainsValue(Output, Element)) { Output.Add(Element); }
						break;
					case ESetOperation::Except:
						if (!ContainsValue(Second, Element) && !ContainsValue(Output, Element)) { Output.Add(Element); }
						break;
					case ESetOperation::Intersect:
						if (ContainsValue(Second, Element) && !ContainsValue(Output, Element)) { Output.Add(Element); }
						break;
					}
				}

				if (Operation == ESetOperation::Concat)
				{
					Output.Append(Second);
				}
				else if (Operation == ESetOperation::Union)
				{
					for (const auto& SecondValue : Second)
					{
						if (!ContainsValue(Output, SecondValue)) { Output.Add(SecondValue); }
					}
				}

				return TryMakeArrayResultFromValues(ArrayProperty, Output, Arguments, Result);
			});
	};

	FunctionList->Add(TEXT("Concat"), MakeSetFunction(ESetOperation::Concat));
	FunctionList->Add(TEXT("Union"), MakeSetFunction(ESetOperation::Union));
	FunctionList->Add(TEXT("Except"), MakeSetFunction(ESetOperation::Except));
	FunctionList->Add(TEXT("Intersect"), MakeSetFunction(ESetOperation::Intersect));

	FunctionList->Add(TEXT("Select"), MakeFunction(DeclaringType,
		[](const FArrayProperty* ArrayProperty, FScriptArrayHelper& ArrayHelper, FFormulaInvokeArguments& Arguments, TSharedPtr<FFormulaValue>& Result) -> bool
		{
			TSharedPtr<FFormulaClosure> Selector;
			if (FindClosureArgument(Arguments, Selector) != EClosureArgument::Found)
			{
				return false;
			}

			TArray<TSharedRef<FFormulaValue>> Projected;
			if (!TryGetAggregateValues(ArrayProperty, ArrayHelper, Arguments, Projected))
			{
				return false;
			}

			// an empty source keeps the source element type, there being nothing to project
			const EFormulaValueType ProjectedTypeCode = Projected.Num() > 0
				? Projected[0]->GetTypeCode()
				: GetFormulaValueTypeCode(ArrayProperty->Inner);

			FArrayProperty* ResultArrayProperty = FindArrayPropertyForTypeCode(ProjectedTypeCode);
			if (!ResultArrayProperty)
			{
				// struct and enum projections have no declared array property to build into
				UE_LOG(LogFormulaEnumerable, Error,
					TEXT("'Select' cannot build a sequence of '%s' because no array type is declared for it. Project to a primitive, string or object instead. An empty sequence is returned."),
					*(Projected.Num() > 0 ? Projected[0]->GetCPPType() : FFormulaValue::GetExtendedCppName(ArrayProperty->Inner)));
				return TryMakeArrayResult(ArrayProperty, ArrayHelper, TArray<int32>(), Result);
			}

			const TSharedRef<FFormulaValue> NewArray = MakeShared<FFormulaValue>(static_cast<FProperty*>(ResultArrayProperty));
			void* NewArrayPtr = nullptr;
			if (!NewArray->TryGetContainerAddress(NewArrayPtr) || !NewArrayPtr)
			{
				return false;
			}

			FScriptArrayHelper DestinationHelper(ResultArrayProperty, NewArrayPtr);
			for (const auto& ProjectedValue : Projected)
			{
				const int32 AddedIndex = DestinationHelper.AddValue();
				if (!ProjectedValue->TryCopyCompleteValue(ResultArrayProperty->Inner, DestinationHelper.GetElementPtr(AddedIndex)))
				{
					UE_LOG(LogFormulaEnumerable, Error,
						TEXT("'Select' could not store a projected '%s' into '%s', so the default is used for this element."),
						*ProjectedValue->GetCPPType(), *FFormulaValue::GetExtendedCppName(ResultArrayProperty));
					ResultArrayProperty->Inner->ClearValue(DestinationHelper.GetElementPtr(AddedIndex));
				}
			}

			Result = NewArray;
			return true;
		}));
}
