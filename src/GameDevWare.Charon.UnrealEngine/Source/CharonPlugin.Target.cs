// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class CharonPluginTarget : TargetRules
{
	public CharonPluginTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		ExtraModuleNames.Add("RpgGameData");
		ExtraModuleNames.Add("TestData");
		ExtraModuleNames.Add("Charon");
		ExtraModuleNames.Add("CharonPluginGame");
	}
}
