// Copyright GameDevWare, Denis Zykov 2024

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/*
 * Charon-related module with base classes related to game data loading and formatting and formulas functionality.
 */
class CHARON_API FCharonModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
