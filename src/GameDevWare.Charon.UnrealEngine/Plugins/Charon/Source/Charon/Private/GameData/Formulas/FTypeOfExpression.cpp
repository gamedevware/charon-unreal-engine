#include "GameData/Formulas/Expressions/FTypeOfExpression.h"
#include "GameData/Formulas/FExpressionBuildHelper.h"
#include "GameData/Formulas/FFormulaNotation.h"
#include "GameData/Formulas/IFormulaTypeDescription.h"

FTypeOfExpression::FTypeOfExpression(const TSharedRef<FJsonObject>& ExpressionObj):
	TypeRef(FExpressionBuildHelper::GetTypeRef(ExpressionObj, FFormulaNotation::TYPE_ATTRIBUTE))
{}

FFormulaInvokeResult FTypeOfExpression::Execute(const FFormulaExecutionContext& Context) const
{
	if (!this->TypeRef.IsValid())
	{
		return FFormulaInvokeError::ExpressionIsInvalid();
	}
	
	const auto ToType = Context.TypeResolver->GetTypeDescription(this->TypeRef);
	if (!ToType)
	{
		return FFormulaInvokeError::UnableToResolveType(this->TypeRef->GetFullName(/* include generics */ true));
	}
	
	return MakeShared<FFormulaValue>(static_cast<UObject*>(ToType->GetTypeClassOrStruct()));
}
