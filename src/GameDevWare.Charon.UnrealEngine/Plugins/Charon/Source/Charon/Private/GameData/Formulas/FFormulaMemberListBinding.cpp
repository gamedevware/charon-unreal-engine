#include "GameData/Formulas/Expressions/FFormulaMemberListBinding.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/FFormulaProperty.h"
#include "GameData/Formulas/Expressions/FFormulaExpression.h"

FFormulaMemberListBinding::FFormulaMemberListBinding(const TSharedRef<FJsonObject>& ExpressionObj) :
	FFormulaMemberBinding(RawMemberName),
	Initializers(FExpressionBuildHelper::GetArgumentsList(ExpressionObj, FFormulaNotation::ARGUMENTS_ATTRIBUTE))
{
}

FFormulaMemberListBinding::FFormulaMemberListBinding(const FString& RawMemberName,
                                                     const TArray<TSharedPtr<FFormulaExpression>>& Initializers) :
	FFormulaMemberBinding(RawMemberName),
	Initializers(Initializers)
{
}

bool FFormulaMemberListBinding::IsValid() const
{
	if (this->RawMemberName.IsEmpty() || this->MemberName.IsEmpty())
	{
		return false;
	}
	for (auto Initializer : this->Initializers)
	{
		if (!Initializer.IsValid())
		{
			return false;
		}
	}
	return true;
}

FFormulaExecutionResult FFormulaMemberListBinding::ApplyMemberChanges(const TSharedRef<FFormulaValue>& Target,
                                                                      const FFormulaProperty* Member,
                                                                      const FFormulaExecutionContext& Context) const
{
	check(Member);

	TSharedPtr<FFormulaValue> MemberValue;
	if (!Member->TryGetValue(Target, MemberValue))
	{
		return FFormulaExecutionError::MemberAccessFailed(Target->GetCPPType(), this->MemberName);
	}

	FProperty* MemberType = MemberValue->GetType();
	if (const FArrayProperty* ArrayProp = CastField<FArrayProperty>(MemberType))
	{
		void* ArrayAddress;
		check(Target->TryGetContainerAddress(ArrayAddress));
		check(ArrayAddress != nullptr);

		FScriptArrayHelper ArrayWrap(ArrayProp, ArrayAddress);

		for (const auto Initializer : this->Initializers)
		{
			const auto ValueResult = Initializer->Execute(Context, ArrayProp->Inner);
			if (ValueResult.HasError())
			{
				return ValueResult; // propagate error
			}

			const int32 AddedIndex = ArrayWrap.AddUninitializedValue();
			const bool bValueAdded = ValueResult.GetValue()->TryCopyCompleteValue(
				ArrayProp->Inner,
				ArrayWrap.GetElementPtr(AddedIndex)
			);
			if (!bValueAdded)
			{
				ArrayWrap.RemoveValues(AddedIndex, 1);
				return FFormulaExecutionError::CollectionAddFailed(FFormulaValue::GetExtendedCppName(ArrayProp),
				                                                   ValueResult.GetValue()->GetCPPType());
			}
		}
		return Target;
	}
	else if (const FSetProperty* SetProp = CastField<FSetProperty>(MemberType))
	{
		void* SetAddress;
		check(Target->TryGetContainerAddress(SetAddress));
		check(SetAddress != nullptr);

		FScriptSetHelper SetWrap(SetProp, SetAddress);

		for (const auto Initializer : this->Initializers)
		{
			const auto ValueResult = Initializer->Execute(Context, SetProp->ElementProp);
			if (ValueResult.HasError())
			{
				return ValueResult; // propagate error
			}

			const int32 AddedIndex = SetWrap.AddUninitializedValue();
			const bool bValueAdded = ValueResult.GetValue()->TryCopyCompleteValue(
				SetProp->ElementProp,
				SetWrap.GetElementPtr(AddedIndex)
			);
			if (!bValueAdded)
			{
				SetWrap.RemoveAt(AddedIndex, 1);
				return FFormulaExecutionError::CollectionAddFailed(FFormulaValue::GetExtendedCppName(SetProp),
				                                                   ValueResult.GetValue()->GetCPPType());
			}
		}

		return Target;
	}
	else
	{
		return FFormulaExecutionError::UnsupportedArrayType(FFormulaValue::GetExtendedCppName(MemberType));
	}
}

void FFormulaMemberListBinding::DebugPrintTo(FString& OutValue) const
{
	OutValue.Append(this->RawMemberName);
	OutValue.Append(TEXT(": { "));
	bool bFirstInitializer = true;
	for (auto Initializer : this->Initializers)
	{
		if (!bFirstInitializer)
		{
			OutValue.Append(TEXT(", "));
		}
		bFirstInitializer = false;
		
		if (Initializer.IsValid())
		{
			Initializer->DebugPrintTo(OutValue);
		}
		else
		{
			OutValue.Append(TEXT("#INVALID#"));
		}
	}
	OutValue.Append(TEXT("} "));
}
