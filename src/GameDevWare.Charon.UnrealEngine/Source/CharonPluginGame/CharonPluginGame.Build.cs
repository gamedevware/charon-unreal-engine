// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CharonPluginGame : ModuleRules
{
	public CharonPluginGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(new string[] { "GameplayAbilities" });
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "NavigationSystem", "AIModule", "Niagara", "EnhancedInput", "Charon", "TestData" });
    }
}
