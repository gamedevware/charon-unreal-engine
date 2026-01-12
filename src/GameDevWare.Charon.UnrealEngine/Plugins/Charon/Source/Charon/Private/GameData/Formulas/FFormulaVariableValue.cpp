#include "GameData/Formulas/FFormulaVariableValue.h"

FFormulaVariableValue::FFormulaVariableValue()
{
	this->Value.Set<nullptr_t>(nullptr);
	this->Type = EPropertyType::CPT_None;
}
