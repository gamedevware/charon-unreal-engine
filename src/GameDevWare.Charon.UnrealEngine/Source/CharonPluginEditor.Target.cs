// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class CharonPluginEditorTarget : TargetRules
{
	public CharonPluginEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("RpgGameData");
		ExtraModuleNames.Add("TestData");
		ExtraModuleNames.Add("Charon");
		ExtraModuleNames.Add("CharonEditor");
		ExtraModuleNames.Add("CharonPluginGame");
		ExtraModuleNames.Add("CharonPluginGameEditor");
	}
}
