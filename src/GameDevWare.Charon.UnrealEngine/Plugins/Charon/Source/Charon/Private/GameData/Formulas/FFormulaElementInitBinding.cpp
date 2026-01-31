#include "GameData/Formulas/Expressions/FFormulaElementInitBinding.h"

#include "GameData/Formulas/Expressions/FFormulaExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"

FString GetValueTypes(const TArray<TSharedRef<FFormulaValue>>& InValues)
{
	TArray<FString> ValueTypes;
	for (auto Value : InValues)
	{
		ValueTypes.Add(Value->GetCPPType());
	}
	return FString::Join(ValueTypes, TEXT(", "));
}

inline FFormulaElementInitBinding::FFormulaElementInitBinding(const TSharedRef<FJsonObject>& ExpressionObj):
	Initializers(FExpressionBuildHelper::GetArgumentsList(ExpressionObj, FFormulaNotation::INITIALIZERS_ATTRIBUTE))
{
}

FFormulaElementInitBinding::FFormulaElementInitBinding(const TArray<TSharedPtr<FFormulaExpression>>& Initializers):
	Initializers(Initializers)
{
}

bool FFormulaElementInitBinding::IsValid() const
{
	for (auto Initializer : this->Initializers)
	{
		if (!Initializer.IsValid())
		{
			return false;
		}
	}
	return true;
}

FFormulaExecutionResult FFormulaElementInitBinding::Apply(const TSharedRef<FFormulaValue>& Target, const FFormulaExecutionContext& Context) const
{
	constexpr int32 KEY_INDEX = 0;
	constexpr int32 VALUE_INDEX = 1;
	constexpr int32 ELEMENT_INDEX = 0;
	
	if (!this->IsValid())
	{
		return FFormulaExecutionError::ExpressionIsInvalid();
	}
	
	FProperty* CollectionType = Target->GetType();
	TArray<FProperty*> AddTypes;
	if (const FArrayProperty* ArrayProp = CastField<FArrayProperty>(CollectionType))
	{
		AddTypes.Add(ArrayProp->Inner);
	}
	else if (const FSetProperty* SetProp = CastField<FSetProperty>(CollectionType))
	{
		AddTypes.Add(const_cast<FProperty*>(SetProp->GetElementProperty()));
	}
	else if (const FMapProperty* MapProp = CastField<FMapProperty>(CollectionType))
	{
		AddTypes.Add(const_cast<FProperty*>(MapProp->GetKeyProperty()));
		AddTypes.Add(const_cast<FProperty*>(MapProp->GetValueProperty()));
	}
	else
	{
		return FFormulaExecutionError::UnsupportedCollectionType(FFormulaValue::GetExtendedCppName(CollectionType));
	}
	
	TArray<TSharedRef<FFormulaValue>> AddValues;
	for (const auto Initializer : this->Initializers)
	{
		FProperty* AddValueType = AddValues.Num() < AddTypes.Num() ? AddTypes[AddValues.Num()] : nullptr;
		const auto ValueResult = Initializer->Execute(Context, AddValueType);
		if (ValueResult.HasError())
		{
			return ValueResult; // propagate error
		}
		AddValues.Add(ValueResult.GetValue());
	}
	
	if (const FArrayProperty* ArrayProp = CastField<FArrayProperty>(CollectionType))
	{
		if (AddValues.Num() != 1)
		{
			return FFormulaExecutionError::CollectionAddFailed(FFormulaValue::GetExtendedCppName(CollectionType), GetValueTypes(AddValues));
		}
		
		void* ArrayAddress;
		check(Target->TryGetContainerAddress(ArrayAddress));
		check(ArrayAddress != nullptr);
		
		const FProperty* ElementProperty = ArrayProp->Inner;
		FScriptArrayHelper ArrayWrap(ArrayProp, ArrayAddress);
		
		const int32 AddedIndex = ArrayWrap.AddUninitializedValue();
		void* ElementPtr = ArrayWrap.GetElementPtr(AddedIndex);
		ElementProperty->InitializeValue(ElementPtr);

		if (!AddValues[ELEMENT_INDEX]->TryCopyCompleteValue(ElementProperty, ElementPtr))
		{
			ArrayWrap.RemoveValues(AddedIndex);
			return FFormulaExecutionError::CollectionAddFailed(FFormulaValue::GetExtendedCppName(ArrayProp),
			                                                   AddValues[0]->GetCPPType());
		}

		return Target;
	}
	else if (const FSetProperty* SetProp = CastField<FSetProperty>(CollectionType))
	{
		if (AddValues.Num() != 1)
		{
			return FFormulaExecutionError::CollectionAddFailed(FFormulaValue::GetExtendedCppName(CollectionType), GetValueTypes(AddValues));
		}
		
		void* SetAddress;
		check(Target->TryGetContainerAddress(SetAddress));
		check(SetAddress != nullptr);

		const FProperty* ElementProperty = SetProp->GetElementProperty();
		FScriptSetHelper SetWrap(SetProp, SetAddress);
		
		const int32 AddedIndex = SetWrap.AddUninitializedValue();
		void* ElementPtr = SetWrap.GetElementPtr(AddedIndex);
		ElementProperty->InitializeValue(ElementPtr);
		
		if (!AddValues[ELEMENT_INDEX]->TryCopyCompleteValue(ElementProperty, ElementPtr))
		{
			SetWrap.RemoveAt(AddedIndex);
			return FFormulaExecutionError::CollectionAddFailed(FFormulaValue::GetExtendedCppName(SetProp),
															 AddValues[0]->GetCPPType());
		}

		return Target;
	}
	else if (const FMapProperty* MapProp = CastField<FMapProperty>(CollectionType))
	{
		if (AddValues.Num() != 2)
		{
			return FFormulaExecutionError::CollectionAddFailed(FFormulaValue::GetExtendedCppName(CollectionType), GetValueTypes(AddValues));
		}
		void* MapAddress;
		check(Target->TryGetContainerAddress(MapAddress));
		check(MapAddress != nullptr);
		
		const FProperty* KeyProperty = MapProp->GetKeyProperty();
		const FProperty* ValueProperty = MapProp->GetValueProperty();
		FScriptMapHelper MapWrap(MapProp,MapAddress);

		const int32 AddedIndex = MapWrap.AddUninitializedValue();
		void* KeyPtr = MapWrap.GetKeyPtr(AddedIndex);
		void* ValuePtr = MapWrap.GetValuePtr(AddedIndex);
		
		KeyProperty->InitializeValue(KeyPtr);
		ValueProperty->InitializeValue(ValuePtr);
		
		if (!AddValues[KEY_INDEX]->TryCopyCompleteValue(KeyProperty, KeyPtr))
		{
			MapWrap.RemoveAt(AddedIndex);
			return FFormulaExecutionError::CollectionAddFailed(FFormulaValue::GetExtendedCppName(MapProp),
															   AddValues[KEY_INDEX]->GetCPPType());
		}
		
		if (!AddValues[VALUE_INDEX]->TryCopyCompleteValue(ValueProperty, ValuePtr))
		{
			MapWrap.RemoveAt(AddedIndex);
			return FFormulaExecutionError::CollectionAddFailed(FFormulaValue::GetExtendedCppName(MapProp),
															   AddValues[VALUE_INDEX]->GetCPPType());
		}
		
		return Target;
	}
	else
	{
		return FFormulaExecutionError::UnsupportedCollectionType(FFormulaValue::GetExtendedCppName(CollectionType));
	}
}

void FFormulaElementInitBinding::CompleteCollectionInitialization(const TSharedRef<FFormulaValue>& Target)
{
	const FProperty* CollectionType = Target->GetType();
	if (const FSetProperty* SetProp = CastField<FSetProperty>(CollectionType))
	{
		void* SetAddress;
		check(Target->TryGetContainerAddress(SetAddress));
		check(SetAddress != nullptr);

		FScriptSetHelper SetWrap(SetProp, SetAddress);
		
		// rehash Set after changes
		SetWrap.Rehash();
	}
	else if (const FMapProperty* MapProp = CastField<FMapProperty>(CollectionType))
	{
		void* MapAddress;
		check(Target->TryGetContainerAddress(MapAddress));
		check(MapAddress != nullptr);
		
		FScriptMapHelper MapWrap(MapProp,MapAddress);
		
		// rehash Map after changes
		MapWrap.Rehash();
	}
}

void FFormulaElementInitBinding::DebugPrintTo(FString& OutValue) const
{
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
}
