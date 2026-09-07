// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "../EFormulaValueType.h"
#include "../FFormulaClosure.h"

#include "UDotNetFunc.generated.h"

/*
 * Carries an FFormulaClosure as a struct value so that lambdas can travel through
 * FFormulaValue without adding a new EFormulaValueType.
 */
USTRUCT()
struct FDotNetFuncValue
{
	GENERATED_BODY()

	TSharedPtr<FFormulaClosure> Closure;
};

/*
 * Internal struct used for reflection in formulas
 */
UCLASS(Hidden, NotPlaceable, Abstract)
class CHARON_API UDotNetFunc : public UObject
{
	GENERATED_BODY()

private:
	UPROPERTY()
	FDotNetFuncValue __Literal;
public:
	inline static EFormulaValueType TypeCode = EFormulaValueType::Struct;

	static FStructProperty* GetLiteralProperty()
	{
		static FStructProperty* LiteralProperty = nullptr;

		if (LiteralProperty)
		{
			return LiteralProperty;
		}
		return LiteralProperty = CastFieldChecked<FStructProperty>(StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UDotNetFunc, __Literal)));
	}
};
