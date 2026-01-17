#pragma once
#include "FFormulaInvokeError.h"
#include "FFormulaValue.h"
#include "Misc/TVariant.h"

using FFormulaInvokeResult = TVariant<FFormulaValue, FFormulaInvokeError>;